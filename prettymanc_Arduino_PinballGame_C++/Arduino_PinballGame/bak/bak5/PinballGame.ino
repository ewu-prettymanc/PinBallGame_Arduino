/**************************************
  * Colton Prettyman & Travis Morasch  *
  * March 2014                         *
  * Final PinBall Game Project         *
  * Real Time Embedded Systems 496     *
  * Eastern Washington University      *
  * Professor Dr. Paul Schimpf         *
  *************************************/
  
#include <BallyLib.h>

//===============================================================================================================================================================================================================================
// Bally object instance
Bally _bally;

int g_playerScores[5][6];
int g_highScore=0;

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600);
  // Zero out Score and blank display
  initArrayToZero();
  for( int i=0; i < 5; i++)
     setDisplay(i,10);
}

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  _bally.setContSolenoid(2, false);
  int  button; 
  int numberOfPlayers = 0;
  int numberOfCredits = 0;
  int numberOfBalls = 3;
  int currentPlayer = 0;
  int currentPlayerUp = 0;
  int currentPlayerScore=0;
  int currentPlayerReplays = 0;
  
  boolean playOver = false;
  
   _bally.setLamp(12,2, true );//game over lamp on
  
  //init all switches that need to be triggered
  Serial.println("DONE setting to zero");
 
  // Poll the credit switch...it takes 2 points to add one player
  // max of 8 credits and 4 players.
  boolean start = false;

 //********************ADD PLAYERS/CREDITS***************************************************
  while( numberOfPlayers < 4 && !start)
 { 
   if( numberOfPlayers >0 )
   {
        if( _bally.getDebRedge(0,7) )
        {
          _bally.fireSolenoid(6, true);
           numberOfBalls =2;
        } // end if
        
        playFieldPoller(&currentPlayerScore, &playOver);
   } // end if
   
   // There is no players but someone released the ball and it
   // got stuck in the saucer hole...or somewhere
   if( numberOfPlayers==0)
   { 
     playFieldPoller(&currentPlayerScore, &playOver );
     // but don't keep the score!! False start
     currentPlayerScore = 0;
   }
    // if any switch on the board was hit then there must be a score incrementation
    // catch it and break our coin adding loop
    if( currentPlayerScore != 0  )
      break;

    // Check for a credit button press...don't let them put more in then
    // they can use!!
    if( getDebRedgeCredit && numberOfCredits < 8-numberOfPlayers*2 )
    {
        numberOfCredits++;
        _bally.setDisplay(4, 3, numberOfCredits);
    } // end if
      
    // Check for a coin (player) add press 
    if( getDebRedgeCoin && numberOfCredits >= 2 )
    {
        _bally.setLamp(13, numberOfPlayers, true);
        numberOfPlayers++;
        numberOfCredits -= 2;
        _bally.setDisplay(4, 3, numberOfCredits);  
    } // end if     
 }// end coin/credit while 
 //**************************END ADD PLAYERS/CREDITS**************************
 
 //**************************START GAME*******************************************************
   _bally.setLamp(12,2, false );//turn off game over lamp
   
 // Place the player scores to the display...0'd the first round
 for(int j=0; j < numberOfPlayers; j++)
   for(int i = 0; i != 7; i++ )
       _bally.setDisplay(j,i, g_playerScores[j][i]);// change 0 to player number
  
  // For each player give 3 balls to play...
  // NOTE: Scores are latched to the displays until reset.
  for(; numberOfPlayers != 0; numberOfPlayers--)
  {
    for(; numberOfBalls !=-1; numberOfBalls--)
    {
        _bally.zeroSwitchMemory();
        _bally.setLamp(14,currentPlayerUp, true );//player up  
        _bally.setLamp(12,0, true );// ball in play lamp
        _bally.setDisplay(4, 1, 0);
        _bally.setDisplay(4, 0, numberOfBalls);
        
         // up drop targests
       _bally.fireSolenoid(3, true, true);
       _bally.fireSolenoid(7, true, true);
     
       _bally.fireSolenoid(6, true);//kick ball out
       _bally.zeroSwitchMemory();
       playOver = false;
       
       while( !playOver )
       {
         // Let this function take care of all our switch polling and action
         playFieldPoller(&currentPlayerScore, &playOver);
         parseToDisplay( currentPlayerUp, currentPlayerScore);
       }//End While
       
       // Deal with high Scores and bonus balls. Replay same thing??
       if( (currentPlayerScore > 200000 && currentPlayerReplays == 0)  ||
           ( currentPlayerScore > 340000 && currentPlayerReplays == 1) ||
           ( currentPlayerScore >450000 && currentPlayerReplays == 2 ) )
       {
         // Turn on the extra ball light
         _bally.setLamp(6,1, true);
         numberOfBalls++;
       }
       
       // They can play again...turn on the shoot again lamps
       if( numberOfBalls != 0 )
       {
         _bally.setLamp(10,0, true);
         _bally.setLamp(10,2, true);
       }
       
      delay(500);
    }//end for numberOfBalls
    
    // Flash the player ball lights on and off and chime
    for( int i; i < 6; i++)
    {
      if( i%2 == 1 )
      { // Turn off
        _bally.setDisplay(4,1,10);
        _bally.setDisplay(4,0,10);
      }
      else
      { // Turn on
        _bally.setDisplay(4,1,0);
        _bally.setDisplay(4,0,0);
      }
      
       _bally.fireSolenoid(8, true);
       
      delay(300);
    } // end flash ball used up
    
    delay(2000);
    
    // If the new Score is the new high Score flash that score on the display
    if( currentPlayerScore > g_highScore )
    {
      g_highScore = currentPlayerScore;
      
      for( int i=0; i < 6; i++)
      {
        if( i%2 == 1 )
          setDisplay(currentPlayerUp, 10); 
        else
          parseToDisplay(currentPlayerUp, currentPlayerScore);
          
        _bally.fireSolenoid(2,true);
        delay( 300 );
      } // end for 
    } // end if new high score
    
    // If there are several players and not all have gone already
    if( numberOfPlayers > 1 && currentPlayerUp < 4)
    { // reset so another player can play
       currentPlayerUp++;
       currentPlayerScore = 0;
       numberOfBalls = 2; 
       currentPlayerReplays = 0;
       _bally.setDisplay(4, 0, numberOfBalls);
    } // end if
  }// end for numberOfPlayers
  //********************************END GAME*********************************
}//end Loop


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void playFieldPoller( int*score,  boolean* playover)
{
  // put the params into easier form
  int currentPlayerScore = *score;
  boolean playOver = *playover;
  
  static int aLane1 = 0;
  static int aLane2 = 0;
  static int bLane1 = 0;
  static int bLane2 = 0;
  int startScore = currentPlayerScore;
  
//**************************************START ROW 1******************************************
      static int targetCount = 0;
      unsigned char tmpRowData = _bally.getDebRedgeRow(2);
      
       // Count the number of toggled bits...bit index does not matter...all worth 500 points
       for(; tmpRowData != 0; tmpRowData=tmpRowData>>1)
       { // if the bit is 0 it won't add any score...if 1 it will add another 500
         targetCount += tmpRowData & 1;
        // currentPlayerScore += ((tmpRowData)&1)*500;
         _bally.fireSolenoid(2, true);
       } // end if
       
       currentPlayerScore += targetCount*500;
       
       if(targetCount == 8)
       {
         _bally.fireSolenoid(3, true, true);
         _bally.fireSolenoid(7, true, true);
         targetCount = 0;
         _bally.fireSolenoid(4, true);//chimes
         _bally.fireSolenoid(12, true);
         _bally.fireSolenoid(2, true);
       } // end if
       //need to check if all down
//************************END ROW 2***************************************

//**************************************START ROW 2******************************************
       tmpRowData = _bally.getDebRedgeRow(3);//row three switches
       if(tmpRowData == 1 || tmpRowData == 2)//right and left feed lanes
       {
         currentPlayerScore += 500;
         _bally.fireSolenoid(2, true);
       } // end if
       if(tmpRowData == 4)//drop target rebound
       {
         currentPlayerScore += 100;
         _bally.fireSolenoid(2, true);
       } // end if
       
       if(tmpRowData == 64 || tmpRowData == 16 )//A lane
       {
          _bally.setLamp(3, 3,true );
          if(tmpRowData==16)
             aLane1 = 1;
          if(tmpRowData==64)
             aLane2 = 1;
          currentPlayerScore += 50;
          _bally.fireSolenoid(2, true);
       } // end if
       
       if(tmpRowData == 8 || tmpRowData == 32 )//B Lane
       {
          _bally.setLamp(3, 2,true );
          if(tmpRowData==8)
            bLane1 = 1;
          if(tmpRowData==32)
            bLane2 = 1;
          currentPlayerScore += 50;
          _bally.fireSolenoid(2, true);
       } // end if
       
       if(aLane1 == 1 && bLane1 == 1 && aLane2 == 1 && bLane2 == 1)//A and B lane bonus, needs work
       {
           currentPlayerScore += 1000;
           _bally.setLamp(3, 2,false );
           _bally.setLamp(3, 3,false );
           //DO Some BONUS
           aLane1 = 0;
           aLane2 = 0;
           bLane1 = 0;
           bLane2 = 0;
       } // end if
       
       if(tmpRowData == 128)
       {
         currentPlayerScore += 300;
     
         _bally.fireSolenoid(12, true);
         // Always a multiple of 3 times for the 3 chimes. A random amount though
         for(int i; i < 3*(currentPlayerScore%10); i++) // Top kick out hole "saucer"
         {
           // alternate chimes
           if( i % 3 == 0 )
           _bally.fireSolenoid(2,true);
           else if( i% 3 == 1 )
           _bally.fireSolenoid(4,true);
           else
           _bally.fireSolenoid(8,true);
           
           delay(100);
         } // end for
         
         _bally.fireSolenoid(0, true);
         
         _bally.fireSolenoid(12,true);
       } // end if
//************************END ROW 3***************************************
       
//**************************************START ROW 4******************************************
       tmpRowData = _bally.getDebRedgeRow(4);//row three switches
       if( tmpRowData == 1 || tmpRowData == 2)
       {
         currentPlayerScore += 1000;
         _bally.fireSolenoid(2, true);
       } // end if
       
       if( tmpRowData == 8 || tmpRowData == 4)//bumper slingshots
       {
         currentPlayerScore += 1000;
         if( tmpRowData == 8 )
         {
            _bally.fireSolenoid(13, true);
            _bally.fireSolenoid(4, true);
         } // end if
         if( tmpRowData == 4 )
         {
            _bally.fireSolenoid(11, true);
            _bally.fireSolenoid(4, true);
         } // end if
       } // end if
       
       if( tmpRowData == 16 || tmpRowData == 32)//bottom bumpers
       {
           currentPlayerScore += 100;
           if(tmpRowData == 32 )
           {
             _bally.fireSolenoid(14, true);
             _bally.fireSolenoid(8, true);//chime
             _bally.setLamp(8, 3, true);
           } // end if
           if(tmpRowData==16)
           {
             _bally.fireSolenoid(5, true);
             _bally.fireSolenoid(8, true);
             _bally.setLamp(8, 2, true);
           }// end if
       } // end if
       
       if( tmpRowData == 128 || tmpRowData == 64)//top bumpers
       {
           currentPlayerScore += 100;
           if(tmpRowData == 128)
           {
              _bally.fireSolenoid(1, true);
              _bally.fireSolenoid(8, true);
           }// end if
           if(tmpRowData == 64)
           {
              _bally.fireSolenoid(9, true);
              _bally.fireSolenoid(8, true);
           }// end if
       }// end if
//************************END ROW 3***************************************

      if( tmpRowData != 0 )
           Serial.println(tmpRowData);
  
      if(_bally.getDebRedge(0,7))//out hole
         playOver = true;
         
      // this means they released the ball and made a score....turn off any
      // pregame lamps such as bonus ball, shoot again...etc. place such 
      // things in this if
      if( startScore > currentPlayerScore )
      {
         _bally.setLamp(6,1, false); // extra ball
         _bally.setLamp(10,0, false); // shoot again
         _bally.setLamp(10,2, false); // shoot again
      }
         
       // "return" all of theses values by placing the data into the pointers
       *score = currentPlayerScore;
       *playover  = playOver;  
}


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
boolean getDebRedgeCredit()
{
  // Returns the Rising Edge Debounced Switch state of the Credit button
  static boolean oldState = false;
  static boolean prevState=false;
  boolean curState= _bally.getCabSwitch(1,0);
  
  if( oldState == false && prevState == true && curState == true)
  {
    oldState = true;
    return true;
  }
  
  // Shift old values
  oldState = prevState;
  prevState = curState;
  
  return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
boolean getDebRedgeCoin()
{
  // Returns the Rising Edge Debounced Switch state of the Credit button
  static boolean oldState = false;
  static boolean prevState=false;
  boolean curState= _bally.getCabSwitch(0,5);
  
  if( oldState == false && prevState == true && curState == true)
  {
    oldState = true;
    return true;
  }
  
  // Shift old values
  oldState = prevState;
  prevState = curState;
  
  return false;
}

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void initArrayToZero()
{
  for( int i =0; i != 5; i++)
  {
     for( int j = 0; j != 6; j++ ) 
       g_playerScores[i][j] = 0;
  } // end for
}// end initArrayToZero

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void parseToDisplay(int player, int score)
{
  int mod = 0;
  
  // Parse the row data into the display
  for(int i=0; i<7; i++)
  {
    mod = score%10;
    score = score/10;
    g_playerScores[player][i] = mod;
  } // end for
  
 //push the parsed display to the bally library on display 0
 for( int i=0; i < 6; i++)
   _bally.setDisplay(player,i, g_playerScores[player][i]);
} // end parseToDisplay

//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
void setDisplay(int player, int val)
{
   for( int j = 0; j < 6; j++ ) 
      _bally.setDisplay(player, j , val);
}// end initArrayToZero
