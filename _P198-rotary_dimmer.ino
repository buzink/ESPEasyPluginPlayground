//#######################################################################################################
//##################### Plugin 198: Rotary encoder (under development) ######################
//#######################################################################################################

//Connect pin 1 and 3 to pull-up pins. Connect pin 2 to GROUND. 

#define PLUGIN_198
#define PLUGIN_ID_198         198
#define PLUGIN_NAME_198       "Rotary encoder"
#define PLUGIN_VALUENAME1_198 "Rotation"

boolean Plugin_198(byte function, struct EventStruct *event, String& string)
{
  char deviceTemplate[8][64];
  boolean success = false;
  volatile int Plugin_198_lastEncoded = 0;
  volatile int Plugin_198_encoderValue = 0;
  static float initlevel = 0;
  static float Plugin_198_encoderValue_last = 0;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
      {
        Device[++deviceCount].Number = PLUGIN_ID_198;
        Device[deviceCount].Type = DEVICE_TYPE_DUAL;
        Device[deviceCount].VType = SENSOR_TYPE_SINGLE;
        Device[deviceCount].Ports = 0;
        Device[deviceCount].PullUpOption = true;
        Device[deviceCount].FormulaOption = false;
        Device[deviceCount].ValueCount = 1;
        Device[deviceCount].SendDataOption = true;
        break;
      }

    case PLUGIN_GET_DEVICENAME:
      {
        string = F(PLUGIN_NAME_198);
        break;
      }

    case PLUGIN_GET_DEVICEVALUENAMES:
      {
        strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_198));
        break;
      }

    case PLUGIN_WEBFORM_LOAD:
      {
        char deviceTemplate[8][64];
        LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        string += F("<TR><TD>Initial value ");
        string += 0 + 1;
        string += F(":<TD><input type='text' size='80' maxlength='80' name='Plugin_198_template");
        string += 0 + 1;
        string += F("' value='");
        string += deviceTemplate[0];
        string += F("'>"); 
      }
      
     case PLUGIN_WEBFORM_SAVE:
      {
        char deviceTemplate[8][64];
        char argc[25];
        String arg = F("Plugin_198_template");
        arg += 0 + 1;
        arg.toCharArray(argc, 25);
        String tmpString = WebServer.arg(argc);
        strncpy(deviceTemplate[0], tmpString.c_str(), sizeof(deviceTemplate[0]));
        
        //Settings.TaskDeviceID[event->TaskIndex] = 5; // temp fix, needs a dummy value

        SaveCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
        success = true;
        break;
      }

    case PLUGIN_INIT:
      {
        pinMode(Settings.TaskDevicePin1[event->TaskIndex], INPUT_PULLUP);
        pinMode(Settings.TaskDevicePin2[event->TaskIndex], INPUT_PULLUP);
        //attachInterrupt(Settings.TaskDevicePin1[event->TaskIndex], Plugin_198_interrupt1, FALLING);
        success = true;
        break;
      }

    case PLUGIN_TEN_PER_SECOND:
      {
          char deviceTemplate[8][64];
          LoadCustomTaskSettings(event->TaskIndex, (byte*)&deviceTemplate, sizeof(deviceTemplate));
          String tmpString = "[Import1#tablelamp]"; //deviceTemplate[0];
          if (tmpString.length())
          {
            String newString = parseTemplate(tmpString, 20);
            initlevel = newString.toInt();
          } 
          else
          {
            initlevel = -1;       
          }
          if ( 0 >= initlevel <= 100) { Plugin_198_encoderValue = initlevel; } else {Plugin_198_encoderValue = Plugin_198_encoderValue_last;}
          Plugin_198_encoderValue_last = Plugin_198_encoderValue;
          int starttime = millis();
          int endtime = starttime;
          while ((endtime - starttime) <=100) // do this loop for up to 1000mS
          {
            int Plugin_198_MSB = digitalRead(Settings.TaskDevicePin1[event->TaskIndex]); //MSB = most significant bit
            int Plugin_198_LSB = digitalRead(Settings.TaskDevicePin2[event->TaskIndex]); //LSB = least significant bit

            int Plugin_198_encoded = (Plugin_198_MSB << 1) |Plugin_198_LSB; //converting the 2 pin value to single number
            int Plugin_198_sum  = (Plugin_198_lastEncoded << 2) | Plugin_198_encoded; //adding it to the previous encoded value
          
            Plugin_198_lastEncoded = Plugin_198_encoded; //store this value for next time    

            if (Plugin_198_sum == 0b1101 || Plugin_198_sum == 0b0100 || Plugin_198_sum == 0b0010 || Plugin_198_sum == 0b1011)
            { 
              Plugin_198_encoderValue ++; //CW
            }
            if (Plugin_198_sum == 0b1110 || Plugin_198_sum == 0b0111 || Plugin_198_sum == 0b0001 || Plugin_198_sum == 0b1000)
            {
              Plugin_198_encoderValue --; //CCW
            }
            endtime = millis();
          }
          if (Plugin_198_encoderValue > 100) {Plugin_198_encoderValue = 100;}
          if (Plugin_198_encoderValue < 0 ) {Plugin_198_encoderValue = 0;}
          if ( (0 >= Plugin_198_encoderValue <= 100) && (Plugin_198_encoderValue_last != Plugin_198_encoderValue))
          {
            UserVar[event->BaseVarIndex] = Plugin_198_encoderValue;
            sendData(event);
            success = true;
          }  else {
            break;
          }
      }

    case PLUGIN_READ:
      {
        // We do not actually read the pin state as this is already done 10x/second
        // Instead we just send the last known state stored in Uservar
        String log = F("RO   : State ");
        log += UserVar[event->BaseVarIndex];
        addLog(LOG_LEVEL_INFO, log);
        success = true;
        break;
      }
  }
  return success;
}
