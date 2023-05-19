#include "seeed_line_chart.h"  // library for drawing the burndown chart
#include <map>
#include <ArduinoJson.h>  // json library
#include "Seeed_FS.h"     // SD card library
#include "UserInformation.h"

UserInformation userInformation(67, 175, 23, 0);  // (userWeight, userHeight, userAge, isMale)

#include "MotionDetection.h"
#include "MusicPlayer.h"

MusicPlayer player(2);

#include "Scenes.h"
Scenes scenes;
#include "BurndownChart.h"
#include "MqttConnection.h"
#include "ButtonHandler.h"
ButtonHandler button;
float movementValue;

MotionDetection motionDetection;
BurndownChart burndownChart;

void setup() {
  Serial.begin(9600);  // Start serial communication
  setupMqtt();
  button.setup();

  while (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI)) {  // setup sd
    Serial.print("ERROR sd card not recognized");
  }

  motionDetection.startAccelerator();
  burndownChart.initializeUI();
  burndownChart.updateGraphVizuals(); //this is here to start burndownchart in the background
}

void loop() {
  loopMqtt();

  // Exercise isn't complete yet: Continually read movement-values and update chart accordingly
  if (burndownChart.isExercising()) { // burndownChart.isExercising()

    burndownChart.controlConstraints();
    button.onPress();
    button.menuNavigationOnPress(showPlayerScene, showBurndownChartScene);

    motionDetection.recordPreviousAcceleration();  // Read previous user-position
    runMusicPlayer();
    registerChartValues();

    client.publish(calorie_pub, String(burndownChartBackEnd.getCaloriesBurnt()).c_str());
  }

  // Exercise is completed: Inactivate burndown chart and show panel
  else {
    burndownChart.displayExerciseResults();
    delay(200); // to make the scene flicker less
  }
}

void registerChartValues()
{
  burndownChart.updateTimeElapsed();
  movementValue = motionDetection.detectMotion();  // Read current user-position
  burndownChart.sufficientMovementInquiry(userInformation, movementValue);
}

void requestNewNotes()
{
  Serial.println(player.getPosition());
  client.publish(Request_pub, "I need a new set of notes");
  player.hasRequested = true;
}

void runMusicPlayer()
{
  if (player.song.size() > 0 && !player.hasRequested) {
    if (player.getPosition() >= player.song.size()) {
      requestNewNotes();
    } else {
      player.playChunk();
    }
  } else {
    delay(burndownChart.getUpdateDelay());
  }
}

void showBurndownChartScene() {
  burndownChart.updateGraphVizuals();
}

void showPlayerScene() {
  scenes.playerScene();
}