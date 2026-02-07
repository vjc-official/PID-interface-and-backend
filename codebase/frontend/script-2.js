//SCRIPT FOR AUTO PAGE

//FOR DEVICE NAME
const deviceName = document.getElementById("deviceName");

//FOR VALUES OF Rise Time, Settling Time, Steady State Error, Overshoot
const riseTimeValue = document.getElementById("riseTimeValue");
const settlingTimeValue = document.getElementById("settlingTimeValue");
const steadyStateErrorValue = document.getElementById("steadyStateErrorValue");
const overshootValue = document.getElementById("overshootValue");

//FOR VALUES OF Kp, Ki, and Kd
const proportionalGainValue = document.getElementById("proportionalGainValue");
const integralGainValue = document.getElementById("integralGainValue");
const derivativeGainValue = document.getElementById("derivativeGainValue");

//FOR SET ANGLE AND BUTTON
const setAngleValue = document.getElementById("setAngleValue");
const sendDataButton = document.getElementById("sendDataButton");

function sendData() {}

//initiates the sending of data from website to ESP32
sendDataButton.addEventListener("click", sendData);

//Server-side Code
