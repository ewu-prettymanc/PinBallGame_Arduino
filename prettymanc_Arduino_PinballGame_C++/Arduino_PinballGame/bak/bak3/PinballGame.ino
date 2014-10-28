#include <TimerOne.h>
#include <BallyLib.h>

// Bally object instance
Bally _bally;

int playerScores[5][6];

void setup()
{
  Serial.begin(9600);
  // Zero out Score and blank display
  for( int i=0; i < 5; i++)
     for( int j=0; j < 6; j++)
        _bally.setDisplay(i,j, 10 );
}

void loop()
{
     _bally.setContSolenoid(2, false);
 int  button; 
  initArrayToZero(); 
  for( int i =0; i != 5; i++)
  {
     for( int j = 0; j != 6; j++ ) 
     {
       Serial.print(playerScores[i][j]);
     }
       Serial.print("\n");
  }
  int numberOfPlayers = 0;
  int numberOfCredits = 0;
  int currentPlayer = 0;//default to player 1
  
   int currentPlayerUp = 0;
   int currentPlayerScore=0;
   int aLane = 0;
   int bLane = 0; 
   boolean playOver = false;
  //drop target counters?
   _bally.setLamp(12,2, true );//game over lamp on
  
  //init all switches that need to be triggered
  Serial.println("DONE setting to zero");
 
 // Poll the credit switch...it takes 2 points to add one player
 // max of 8 credits and 4 players.
 boolean start = false;

while( numberOfPlayers < 4 && !start)
 { 
   if( numberOfPlayers >0 )
      switchPoller(&currentPlayerUp, &currentPlayerScore, &aLane, &bLane,&playOver);
    // if any switch on the board was hit then there must be a score incrementation
    // catch it and break our coin adding loop
    if( currentPlayerScore != 0  )
      break;
      
   // button = _bally.waitForTestCreditCoin(0,5,1,0);
    //Serial.println(button); 
    // Check for a credit button press...don't let them put more in then
    // they can use!!
    if( _bally.getCabSwitch(1,0) && numberOfCredits < 8-numberOfPlayers*2 )
    {
      delay(100);
        if( _bally.getCabSwitch(1,0) )
        {
          numberOfCredits++;
         _bally.setDisplay(4, 3, numberOfCredits);
        }
    }
      
    // Check for a coin (player) add press 
    if( _bally.getCabSwitch(0,5) && numberOfCredits >= 2 )
    {
      delay(150);
      if( _bally.getCabSwitch(0,5) )
      {
      _bally.setLamp(13, numberOfPlayers, true);
      numberOfPlayers++;
      numberOfCredits -= 2;
       _bally.setDisplay(4, 3, numberOfCredits);  
      }   
    }      
 }
 
 
 //////////////START GAME///////////////////////////
 
   _bally.setLamp(12,2, false );//turn off game over lamp

   
   for(int j=0; j < numberOfPlayers; j++)
     for(int i = 0; i != 7; i++ )
       _bally.setDisplay(j,i,  playerScores[j][i]);// change 0 to player number
       
  for(int NumberofBalls = 3; NumberofBalls !=0; NumberofBalls--)
  {
      _bally.zeroSwitchMemory();
      _bally.setLamp(14,currentPlayerUp, true );//player up  
      _bally.setLamp(12,0, true );// ball in play lamp
      _bally.setDisplay(4, 1, 0);
      _bally.setDisplay(4, 0, NumberofBalls);
        
         // up drop targests
     _bally.fireSolenoid(3, true, true);
     _bally.fireSolenoid(7, true, true);
     
     _bally.fireSolenoid(6, true);
     _bally.zeroSwitchMemory();
     playOver = false;
     while( ! playOver )
     {
       
         // Let this function take care of all our switch polling and action
           switchPoller(&currentPlayerUp, &currentPlayerScore, &aLane, &bLane,&playOver);
           parseToDisplay( currentPlayerUp, currentPlayerScore);
     }//End While
     
     //reset ball count and current play up and other stuff
    // while(_bally.waitForTestCreditCoin(0,5,1,0) != 16);
    
  }//End For  
   
   
   while(1);
}


//-----------------------------------------------------------------------------------------
int switchPoller(int* player, int*score, int* alane, int* blane,  boolean* playover)
{
  // put the params into easier form
  int currentPlayerUp = *player;
  int currentPlayerScore = *score;
  int aLane = *alane;
  int bLane = *blane;
  boolean playOver = *playover;
  
  // for each switch hit take appropriate action.
///////////////////////////ROW 2///////////////////////////
       int targetCount = 0;
       unsigned char tmpRowData = _bally.getDebRedgeRow(2);
        if(tmpRowData != 0)
        {
          targetCount++;
          Serial.print("t :");
          Serial.println(targetCount);
        }
        // Count the number of toggled bits...bit index does not matter...all worth 500 points
       for(; tmpRowData != 0; tmpRowData=tmpRowData>>1)
       { // if the bit is 0 it won't add any score...if 1 it will add another 500
          currentPlayerScore += ((tmpRowData>>0)&1)*500;
          _bally.fireSolenoid(2, true);
        }
       if(targetCount == 8)
       {
          _bally.fireSolenoid(3, true, true);
         _bally.fireSolenoid(7, true, true);
         targetCount = 0;
         _bally.fireSolenoid(4, true);//chimes
         _bally.fireSolenoid(12, true);
         _bally.fireSolenoid(2, true);
       }
       //need to check if all down
///////////////////////////ROW 3///////////////////////////
       tmpRowData = _bally.getDebRedgeRow(3);//row three switches
       if(tmpRowData == 1 || tmpRowData == 2)//right and left feed lanes
       {
         currentPlayerScore += 500;
         _bally.fireSolenoid(2, true);
       }
       if(tmpRowData == 4)//drop target rebound
       {
         currentPlayerScore += 100;
         _bally.fireSolenoid(2, true);
       }
       if(tmpRowData == 64 || tmpRowData == 16 )//A lane
       {
          _bally.setLamp(3, 3,true );
          if(tmpRowData==16)
             aLane++;
          if(tmpRowData==64)
             aLane++;
          currentPlayerScore += 50;
          _bally.fireSolenoid(2, true);
       }
       if(tmpRowData == 8 || tmpRowData == 32 )//B Lane
       {
          _bally.setLamp(3, 2,true );
          if(tmpRowData==8)
            bLane++;
          if(tmpRowData==32)
            bLane++;
          currentPlayerScore += 50;
          _bally.fireSolenoid(2, true);
       }
       if(aLane == 2 && bLane == 2)//A and B lane bonus, needs work
       {
         currentPlayerScore += 1000;
         _bally.setLamp(3, 2,false );
         _bally.setLamp(3, 3,false );
         //DO Some BONUS
         aLane = 0;
         bLane = 0;
       }
       if(tmpRowData == 128)
       {
         currentPlayerScore += 300;
         _bally.fireSolenoid(0, true);
         _bally.fireSolenoid(12, true);
       }
       
///////////////////////////ROW 4///////////////////////////
       tmpRowData = _bally.getDebRedgeRow(4);//row three switches
       if( tmpRowData == 1 || tmpRowData == 2)
       {
         currentPlayerScore += 1000;
         _bally.fireSolenoid(2, true);
       }
       if( tmpRowData == 8 || tmpRowData == 4)//bumper slingshots
       {
         currentPlayerScore += 1000;
         if( tmpRowData == 8 )
         {
            _bally.fireSolenoid(13, true);
            _bally.fireSolenoid(4, true);
         }
         if( tmpRowData == 4 )
         {
            _bally.fireSolenoid(11, true);
            _bally.fireSolenoid(4, true);
         }
       }
       if( tmpRowData == 16 || tmpRowData == 32)//bottom bumpers
       {
         currentPlayerScore += 100;
         if(tmpRowData == 32 )
         {
           _bally.fireSolenoid(14, true);
           _bally.fireSolenoid(8, true);//chime
           _bally.setLamp(8, 3, true);
         }
         if(tmpRowData==16)
         {
           _bally.fireSolenoid(5, true);
           _bally.fireSolenoid(8, true);
           _bally.setLamp(8, 2, true);
         }
       }
       if( tmpRowData == 128 || tmpRowData == 64)//top bumpers
       {
         currentPlayerScore += 100;
         if(tmpRowData == 128)
         {
            _bally.fireSolenoid(1, true);
            _bally.fireSolenoid(8, true);
         }
         if(tmpRowData == 64)
         {
            _bally.fireSolenoid(9, true);
            _bally.fireSolenoid(8, true);
         }
       }
       
           //Serial.println(currentPlayerScore);
           if( tmpRowData != 0 )
           {
               Serial.println(tmpRowData);
           }
      
           if(_bally.getDebRedge(0,7))
             playOver = true;
             
             // "return" all of theses values by placing the data into the pointers
             *player =  currentPlayerUp;
             *score = currentPlayerScore;
             *alane=  aLane;
             *blane = bLane;
             *playover  = playOver;
}
//-----------------------------------------------------------------------------------------
void parseToDisplay(int player, int score)
{
  int mod = 0;
  
  // Parse the row data into the display
  for(int i=0; i<7; i++)
  {
    mod = score%10;
    score = score/10;
    playerScores[player][i] = mod;
  } // end for
  
 //push the parsed display to the bally library on display 0
 for( int i=0; i < 6; i++)
   _bally.setDisplay(player,i, playerScores[player][i]);
} // end parseToDisplay

void initArrayToZero()
{
  for( int i =0; i != 5; i++)
  {
     for( int j = 0; j != 6; j++ ) 
       playerScores[i][j] = 0;
  } 
}
