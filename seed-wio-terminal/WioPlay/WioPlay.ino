#include "seeed_line_chart.h"
TFT_eSPI tft;
TFT_eSprite spr = TFT_eSprite(&tft);

#include "LIS3DHTR.h"   //include accelerometer library
#include <map>
LIS3DHTR <TwoWire> lis;  //Initialize accelerometer

using std::string;
#include"TFT_eSPI.h"

#define MAX_SIZE 10 // maximum size of data displayed at once in graph
doubles data[2];

float totalNumberOfSegments; // Number of update segments in graph until goal is reached
float currentSegments = 0;

float graphUIXStartValue = 20; // X-coordinate in terminal for the graph to start at
// ---------------------------------------------------------------------------------------

//                              **CALORIES BURNT**
// ---------------------------------------------------------------------------------------

// Note: Row[i] is equivalent to the (i)th activity
// Note: Retrieve standard-value by getting the average of min and max value
// The minimal and maximal met-values for each physical activity
byte metRanges[3][2] =
{
  {3, 6},  // Walking
  {9, 23}, // Running
  {5, 8}   // Hiking
};

// Movement values required to acquire average/standard MET-value for specified exercise
float standardMovementValues[] =
{
  0.3,  // Walking
  3,    // Running
  1.5   // Hiking
};

float standard;
float minMovement; // Minimal movement required for specific exercise (Deals with cases where user isn't moving enough in accordance with selected exercise)
float maxMovement; // Maximal movement required for specific exercise (Handles the case where user selected 'Walking' but is running in reality)
float proportionalConstant;
float movementValue;

// Calorie reference: "List Of METs Of The Most Popular Exercises", https://betterme.world/articles/calories-burned-calculator/
float caloriesBurned = 0;
bool isExercising = true;

// Settings: Selection of physical user characteristics
float userWeight = 65; // Kilogram
float userHeight = 175; // Centimeters
byte userAge = 23;
bool isMale = 0;

// Selection of current exercise
float exerciseDuration = 30; // Seconds
float caloriesGoal = 100;
byte chosenActivityIdx = 0;

float balanceFactor = 0.35;
float delayValue = 200;

// ---------------------------------------------------------------------------------------

// Timer
byte seconds = 0;
byte minutes = 0;
byte hours = 0;

void setup() {
  Serial.begin(115200);  //Start serial communication
  lis.begin(Wire1);      //Start accelerometer

  //Check whether accelerometer is working
  while (!lis) {
    Serial.println("ERROR accelerometer not working");
  }


  lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);  //Data output rate (5Hz up to 5kHz)
  lis.setFullScaleRange(LIS3DHTR_RANGE_2G);       //Scale range (2g up to 16g)
  tft.begin();

  pinMode(A0, INPUT); // NECESSARY?
  tft.begin();
  tft.setRotation(3);
  spr.createSprite(TFT_HEIGHT, TFT_WIDTH);
  spr.setRotation(3);

  totalNumberOfSegments = (exerciseDuration / (delayValue / 1000)) + 1;

  // NOTE: THIS MIGHT BE REDUNDANT
  tft.setRotation(3); // set the screen orientation
  tft.setTextColor(TFT_BLACK); //set text color
  tft.setTextSize(3); //set text size

  standard = (float)(metRanges[chosenActivityIdx][0] + metRanges[chosenActivityIdx][1]) / 2; // Average of the min and max MET-Values of chosen activity
  proportionalConstant = standard / standardMovementValues[chosenActivityIdx];
  minMovement = (float)metRanges[chosenActivityIdx][0] / proportionalConstant; // Minimal movement required for user to be considered actually doing the selected activity
  maxMovement = (float)metRanges[chosenActivityIdx][1] / proportionalConstant; // Maximal movement boundary
}

void loop()
{
  if (isExercising)
  {
    //                              **LINE CHART**
    // ---------------------------------------------------------------------------------------
    if(data[0].size() >= MAX_SIZE)
    {
      for (uint8_t i = 0; i<2; i++)
      {
        data[i].pop(); // Remove the first read variable
      }
    }

    currentSegments++;

    if (currentSegments == totalNumberOfSegments)
    {
      isExercising = false;
    }

    spr.fillSprite(TFT_WHITE);

        // Settings for the line graph title
    auto header = text(80, 0)
                      .value("Burndown chart")
                      // .align(center)
                      // .valign(vcenter)
                      .width(spr.width())
                      .thickness(2);

    header.height(header.font_height(&spr) * 2); // * 2
    header.draw(&spr); // Header height is the twice the height of the font

    auto headerY = text(15, 18)
                  .value("Calories burned") // Note: New line '\n' doesn't work
                  .backgroud(TFT_WHITE)
                  .color(TFT_RED)
                  .thickness(1);

    headerY.height(headerY.font_height(&spr) * 2);
    headerY.draw(&spr); // Header height is the twice the height of the font

    auto headerX = text(240, 190)
      .value("Time elapsed") // Note: New line '\n' doesn't work
      .backgroud(TFT_WHITE)
      .thickness(1);

    headerX.height(headerX.font_height(&spr) * 2);
    headerX.draw(&spr); // Header height is the twice the height of the font

    float expectedValue = ((currentSegments - 1) / totalNumberOfSegments) * caloriesGoal;

    data[0].push(caloriesBurned);
    data[1].push(expectedValue);

        // Settings for the line graph
    auto content = line_chart(graphUIXStartValue, header.height()); //(x,y) where the line graph begins   auto content = line_chart(20, header.height());
    content
        .height(spr.height() - header.height() * 1.5) // actual height of the line chart
        .width(spr.width() - content.x() * 2)         // actual width of the line chart
        .based_on(0.0)                                // Starting point of y-axis, must be a float
        .show_circle(true, false)
        .value({data[0], data[1]})
        .max_size(MAX_SIZE)
        .color(TFT_RED, TFT_BLUE)
        .backgroud(TFT_WHITE)
        // Thickness
        .draw(&spr);

    spr.pushSprite(0, 0);
    // ---------------------------------------------------------------------------------------

    //                              **CALORIES BURNED**
    // ---------------------------------------------------------------------------------------
    float current_x, current_y, current_z;  //Initialize variables to store accelerometer values
    float prev_x, prev_y, prev_z;

    lis.getAcceleration(&prev_x , &prev_y, &prev_z);

    delay(delayValue);

    lis.getAcceleration(&current_x , &current_y, &current_z);

    float diff_x = abs(current_x - prev_x), diff_y = abs(current_y - prev_y), diff_z = abs(current_z - prev_z);
    float movementValue = diff_x + diff_y + diff_z;

    // User passes minimal movement-requirement for burning calories in selected exercise
    if (movementValue >= minMovement)
    {
      caloriesBurned += burnCalories(getMETValue(movementValue));
    }
    else
    {
      Serial.println("You are not exercising hard enough for the selected exercise!");
    }

    updateTimer();
  }
  else
  {
    tft.fillScreen(TFT_RED);
    tft.setTextSize(3);
    tft.drawString("Menu here!", 50, 70);
  }
}

// Formula reference: "Calculating daily calorie burn", https://www.medicalnewstoday.com/articles/319731
// Takes into consideration the inputted user characteristics and how much the user has moved since last update of the burndown chart to calculate calories burnt
float burnCalories(float movementValue) // Burn calories based on the movement-value
{
  float moveFactor = (movementValue / delayValue) * balanceFactor; // Measurement of how much user has moved since last iteration in 'loop()'

  if (isMale) // Multiply with 'balanceFactor'
  {
    return (66 + (6.2 * userWeight) + (12.7 * userHeight) - (6.76 * userAge)) * moveFactor;
  }
  else
  {
    return (655.1 + (4.35 * userWeight) + (4.7 * userHeight) - (4.7 * userAge)) * moveFactor;
  }
}

float getMETValue(float movementValue)
{
  return movementValue * proportionalConstant;
}

void updateTimer()
{
  seconds++;
  delay(1000);

  if (seconds == 60)
  {
    minutes++;
    seconds = 0;

    if (minutes == 60)
    {
      hours++;
      minutes = 0;
    }
  }
}