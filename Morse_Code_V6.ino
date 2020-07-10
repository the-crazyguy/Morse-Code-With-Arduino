/* Timing:
 * 1 dot = 1 time unit
 * 1 dash = 3 dots' time
 */
#include <LiquidCrystal.h>

//PINS
int btnPin = 7;
int ledPin = 10;
int switchButton = 8;
int buzzerPin = 9;
 
//INTEGERS
const int threshold = 50; //threshold for the minimum time required for the button to be pressed in milliseconds
const int maxDotTime = 500; //the maximum allowed time for entering a dot in milliseconds
int cursorPos = 0;  //the position of the cursor

//SWITCH BUTTON STATES
int switchBtnState = 0;
/* 0 = input
 * 1 = learn alphabet
 * 2 = learn words
 */
 
//TIME PARAMS
unsigned long pushTime; //the time when the button was pressed
unsigned long releaseTime; //the time when the button was released
unsigned long timeLength; //the difference between pushTime and releaseTime
unsigned long timeBetweenLetters = 1000; //the maximum time allowed between letters 

//LCD SCREEN
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//STRINGS/CHARS
String symbolCode = "";

//defines the morse code alphabet for translating
//'A' + *index* = targer letter
String alphabet[] = {".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",
                                ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-",
                                ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..", "END"};

//defines the numbers in morse code
//index = target number
String numbers[] = {"-----", ".----", "..---", "...--", "....-", ".....",
                             "-....", "--...", "---..", "----.", "END"};
 
//defines the most common special cases/characters in morse code
String specialCases[2][7] =
  {
    {" ",".", ",", "?", "!", "SOS", ""},
    {".-.-.", ".-.-.-", "--..--", "..--..", "-.-.--", "...---...", "END"}
  };
 
 
void setup() 
{
  pinMode(switchButton, INPUT_PULLUP);
  pinMode(btnPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
 
  Serial.begin(9600);
}
 
void loop() 
{
  GetSwitchBtnState();
  switch(switchBtnState)
  {
    case 0: 
    {
      //Input
      GetInput(); //Gets the whole character's input
      ConvertToOutput();  //Converts from morse code to letter/number/special symbol
      break;
    }
    case 1:
    {
      //Learn alphabet
      LearnTheAlphabet();
      break;
    }
    case 2:
    {
      //Learn words
      LearnWords();
      lcd.clear();
      break;
    }
  }
}

void GetSwitchBtnState()
{
  if(digitalRead(switchButton) == LOW)
  {
    if(switchBtnState < 2)
    {
      switchBtnState++;
    }
    else
    {
      switchBtnState = 0;
    }
    delay(500); //wait for 0.5 seconds
  }
}

void GetDotDash()
{
  while(digitalRead(btnPin) == HIGH) 
  {
    if(digitalRead(switchButton) == LOW)
    { 
      GetSwitchBtnState();
      return;
    }
  }  
  pushTime = millis();  //time of the button's press
  digitalWrite(ledPin, HIGH); //turns on the led
  tone(buzzerPin, 1000);  //play a sound at a frequency of 1000 Hz

  while(digitalRead(btnPin) == LOW) {} 
  
  releaseTime = millis(); //time of the button's release 
  digitalWrite(ledPin, LOW);  //turns off the led
  noTone(buzzerPin);  //stops playing sound
  
  timeLength = releaseTime - pushTime;
 
  if(timeLength > threshold)
  {
    symbolCode += GetSymbol();
  }
}

void GetInput()
{
  GetDotDash(); //Gets a single dot or dash from the user

  //Keep getting new input of dots/dashes unless timeBetweenLetters (in ms) has elapsed
  while((millis() - releaseTime) < timeBetweenLetters)
  {
    if(digitalRead(switchButton) == LOW)
    {
      GetSwitchBtnState();
      return;
    }
    if(digitalRead(btnPin) == LOW)
    {
      GetDotDash();
    }
  }
  //Input has finished
}

 
char GetSymbol()
{
  if(timeLength <= maxDotTime && timeLength > threshold)
  {
    return '.';
  }
  else if(timeLength > maxDotTime)
  {
    return '-';
  }
}

void ConvertToOutput()
{
  int i = 0;
  char output = '*';
  
  while(alphabet[i] != "END")
  {
    if(alphabet[i] == symbolCode)
    {
      //Convert to string if you want to return it as a result
      output = (char) ('A' + i);
      Serial.print(output);
      lcd.print(output);
      cursorPos++;  //sets the new cursor position
      symbolCode = "";  //resets the symbol code
      
      return; //exits out of the function
    }
    i++;
  }
 
  i = 0;
  while(numbers[i] != "END")
  {
    if(numbers[i] == symbolCode)
    {
      //Convert to string if you want to return it as a result
      Serial.print(i);
      lcd.print(i);
      cursorPos++;  //sets the new cursor position
      symbolCode = "";  //resets the symbol code
 
      return; //exits out of the function
    }
    i++;
  }
 
  i = 0;
  while(specialCases[1][i] != "END")
  {
    if(specialCases[1][i] == symbolCode)
    {
      if(i == 0)
      {
        Serial.print(" ");
        lcd.print(" ");
        cursorPos++;  //sets the new cursor position
      }
      else
      {
        String stringOutput = specialCases[0][i];
        Serial.print(stringOutput);
        lcd.print(stringOutput);
        cursorPos++;  //sets the new cursor position
      }
       
      symbolCode = "";  //resets the symbol code
 
      return; //exits out of the function
    }
    i++;
  }
  if(specialCases[1][i] == "END")
  {
    //The program has searched through everything and a no match was found
    //Either print/return an error or an empty character
    Serial.print("");
    lcd.print("");
    symbolCode ="";
    return;
  }
}


String StringToMorse(String input)
{
  String output = "";
  int inputSize = input.length();
  for (int i = 0; i < inputSize; i++)
  {
     char symbol = input[i]; 
     if(65 <= (int)symbol && 90>= (int)symbol) //Checks if the symbol is a letter
     {
        int index = (int)(symbol-65); //Gets the index of the letter within the alphabet array
        output += alphabet[index]; //Adds the morse code of the letter to the output   
        output += " ";
     }
     else if(48 <= (int)(symbol) && 57 >= (int)(symbol)) //Checks if the symbol is a number
     {
       int index = (int)(symbol)-48; //Gets the index of the number within the numbers array
       output += numbers[index]; //Adds the morse code of the number to the ouput
       output += " ";
     }
     else //If the symbol is neither a letter, nor a number, then it is a special symbol
     {
       if(symbol == ' ')
       {
         output += specialCases[1][0];
         output += " ";
       }
       else if(symbol == '.')
       {
         output += specialCases[1][1];
         output += " ";
       }
       else if(symbol == ',')
       {
         output += specialCases[1][2];
         output += " ";
       }
       else if(symbol == '?')
       {
         output += specialCases[1][3];
         output += " ";
       }
       else if(symbol == '!')
       {
         output += specialCases[1][4];
         output += " ";
       }
       else
       {
         output += "* ";
       }
     }
  }
  return output;
}

//Plays the given Morse code via LED and/or Buzzer
void PlayMorseCode(String morseCode)
{
  int morseCodeLength = morseCode.length();

  for(int i = 0; i < morseCodeLength; i++)
  {
    char symbol = morseCode[i];
    if(symbol == '.')
    {
      //output for a dot's length
      digitalWrite(ledPin, HIGH); //turns on the led
      tone(buzzerPin, 1000);  //play a sound at a frequency of 1000 Hz      
      delay(maxDotTime); //keep the light/buzzer on for 1 dot's time

      digitalWrite(ledPin, LOW);  //turns off the led
      noTone(buzzerPin);  //stops playing sound
      delay(maxDotTime); //keep the light/buzzer off for 1 dot's time 
    }
    else if(symbol == '-')
    {
      //output for a dash's length
      digitalWrite(ledPin, HIGH); //turns on the led
      tone(buzzerPin, 1000);  //play a sound at a frequency of 1000 Hz
      delay(maxDotTime * 3); //keep the light/buzzer on for 3 times the dot's time
      
      digitalWrite(ledPin, LOW);  //turns off the led
      noTone(buzzerPin);  //stops playing sound
      delay(maxDotTime); //keep the light/buzzer off for 1 dot's time 
    }
  }
}

void LearnTheAlphabet()
{
  lcd.clear();
  int index = 0; //index, used for the position in the alphabet array
  char letter = ' '; //The wanted letter
  String originalMorse = "";  //The expected Morse code 
  String userMorse = "";  //The user's Morse code
  bool isCorrect = false; //Checker to see if the user has correctly emulated the Morse code
  static const int pause = 1000; //The pause inbetween attempts/letters in milliseconds (ms)

  do
  {
    letter = 'A' + index;  //gets the letter based on the index
    lcd.clear();
    lcd.print("Look at the led!");
    delay(1000);
    //Show the morse code for the letter with led/buzzer
    originalMorse = alphabet[index];  //gets the morse code for the given letter via the index and the alphabet array
    PlayMorseCode(originalMorse); //produces the Morse code
    lcd.clear();
    lcd.print("Enter "); //print the letter in the console/lcd display
    lcd.print(letter);
    lcd.print(" in morse!");
  
    //Let the user attempt to recreate the Morse code
    GetInput(); //Gets the whole character's input and saves it in the global variable symbolCode
    if (switchBtnState != 1)
    {
      return;
    }
    userMorse = symbolCode; //gets the user-inputted morse code via the global symbolCode variable
    symbolCode = "";  //wipes the global variable for reuse
    
    if(originalMorse == userMorse)
    {
      //Correct
      isCorrect = true;
      lcd.clear();
      lcd.print("Well done!");
      Serial.println("Congratulations!");
      delay(pause);  //Adds a 4 second pause before the next iteration
    }
    else
    {
      //Incorrect
      isCorrect = false;
      lcd.clear();
      lcd.print("Try Again!");
      Serial.println("Try Again!");
      delay(pause);  //Adds a 4 second pause before the next iteration
    }

    //Only increments the index if the user has correctly entered the letter
    if(isCorrect)
    {
      index++;
    }
  }
  while(index < 26);
}

String words[13] = {"YOU","HE","SHE","WE","THEY","AM","IS","ARE","TREE","BIKE","HOME","CAT","DOG"};
int wordsArraySize = 13;

void LearnWords()
{
  lcd.clear();
  int wordIndex = random(0,wordsArraySize);
  String wordToUse = words[wordIndex];
  int indexer = 0; //index, used for the position of the letter of the word
  char letter = ' '; //The wanted letter
  String originalMorse = "";  //The expected Morse code 
  String userMorse = "";  //The user's Morse code
  bool isCorrect = false; //Checker to see if the user has correctly emulated the Morse code
  static const int pause = 1000; //The pause inbetween attempts/letters in milliseconds (ms)
  int wordLength = wordToUse.length();
  
  Serial.println(wordToUse);
  lcd.setCursor(0,0);
  lcd.print(wordToUse); // Prints the word on the first line of the LCD screen
  originalMorse = StringToMorse(wordToUse);
  delay(pause*3);
  String morseArray[wordToUse.length()];
  String outputForArray = "";
  int arrayIndex = 0;
 
  do
  {
    lcd.clear();
    letter = wordToUse[indexer];
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Look at the led!");
    PlayMorseCode(morseArray[indexer]); //produces the Morse code for the current letter
    lcd.clear();
    lcd.print("Enter ");
    lcd.print(letter);
    lcd.print(" in morse!"); //print the letter in the console/lcd display
  
    //Let the user attempt to recreate the Morse code
    Serial.println(switchBtnState);
    GetInput(); //Gets the whole character's input and saves it in the global variable symbolCode
    Serial.println(switchBtnState);
    if (switchBtnState != 2) //Lets the user switch to another mode
    {
      delay(500);
      return;
    }
    Serial.println(switchBtnState);
    userMorse = symbolCode; //gets the user-inputted morse code via the global symbolCode variable
    symbolCode = "";  //wipes the global variable for reuse
    originalMorse = alphabet[(int)(letter) - 65];
    Serial.println(originalMorse);
    Serial.println(userMorse);
    if(originalMorse == userMorse)
    {
      //Correct
      isCorrect = true;
      Serial.println("Correct input!");
      lcd.clear();
      lcd.print("Correct input!");
      delay(pause);  //Adds a 4 second pause before the next iteration
    }
    else
    {
      //Incorrect
      isCorrect = false;
      Serial.println("Incorrect input! Try Again!");
      lcd.clear();
      lcd.print("Try Again!");
      delay(pause);  //Adds a 4 second pause before the next iteration
    }
    Serial.println(indexer);
    //Only increments the index if the user has correctly entered the letter
    if(isCorrect)
    {
      indexer++;
    }
    
  }
  while(indexer < wordLength);
  lcd.clear();
  lcd.print("Congratulations!");
  Serial.println("Congratulations!");
}
