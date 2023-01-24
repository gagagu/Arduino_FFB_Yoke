 #if defined(languageEN)

void LcdPrintIntro() {
  lcd.setCursor(0, 0);
  lcd.print(F("       Arduino      "));
  lcd.print(F("Force-Feedback-Yoke "));
  lcd.print(F("Created by GaGaGu   "));
  lcd.print(F("                v1.2"));
}

void LcdPrintAdjustmendValues() {
  lcd.setCursor(0, 0);
  lcd.print(F("Roll  Force:        "));
  lcd.setCursor(13, 0);
  lcd.print(adjustments_input[ADJ_POTI_ROLL_FORCE_MAX]);

  lcd.setCursor(0, 1);
  lcd.print(F("  PWM Mi-Ma:        "));
  lcd.setCursor(13, 1);
  lcd.print(adjustments_input[ADJ_POTI_ROLL_PWM_MIN]);
  lcd.print(F("-"));
  lcd.print(adjustments_input[ADJ_POTI_ROLL_PWM_MAX]);
    
  lcd.setCursor(0, 2);
  lcd.print(F("Pitch Force:        "));
  lcd.setCursor(13, 2);
  lcd.print(adjustments_input[ADJ_POTI_PITCH_FORCE_MAX]);
  
  lcd.setCursor(0, 3);
  lcd.print(F("  PWM Mi-Ma:        "));
  lcd.setCursor(13, 3);
  lcd.print(adjustments_input[ADJ_POTI_PITCH_PWM_MIN]);
  lcd.print(F("-"));
  lcd.print(adjustments_input[ADJ_POTI_PITCH_PWM_MAX]);
}


void LcdPrintCalibrationMiddle(){
  lcd.setCursor(0, 0);
  lcd.print(F("Put all axes in the "));
  lcd.print(F("middle position and "));
  lcd.print(F("press the calibratib"));
  lcd.print(F("ration button       "));
}

void LcdPrintCalibrationAxesStart(){
  lcd.setCursor(0, 0);
  lcd.print(F("                    "));  
  lcd.print(F("Please move all axes"));
  lcd.print(F("to the end positions"));
  lcd.print(F("                    "));  
}

void LcdPrintCalibrationAxesUpdate(){
  lcd.setCursor(0, 0);
  lcd.print(F("Left  end:          ")); //Left  end
  if (!calibration_mode_roll_min) {
    lcd.setCursor(11, 0);
    lcd.print(poti_roll_min);
  }
  lcd.setCursor(0, 1);
  lcd.print(F("Right end:          ")); //right end
  if (!calibration_mode_roll_max) {
    lcd.setCursor(11, 1);
    lcd.print(poti_roll_max);
  }
  lcd.setCursor(0, 2);
  lcd.print(F("Up    end:          ")); // up end
  if (!calibration_mode_pitch_max) {
    lcd.setCursor(11, 2);
    lcd.print(poti_pitch_max);
  }
  lcd.setCursor(0, 3);
  lcd.print(F("Down  end:          ")); // down end
  if (!calibration_mode_pitch_min) {
    lcd.setCursor(11, 3);
    lcd.print(poti_pitch_min);
  }
 
}
#endif
