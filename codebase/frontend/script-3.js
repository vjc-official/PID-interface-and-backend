//SCRIPT FOR MANUAL PAGE

//For device name
const deviceName = document.getElementById("deviceName");

//For Values of Kp, Ki, Kd, Set Angle, and Button
const proportionalGainValue = document.getElementById("proportionalGainValue");
const integralGainValue = document.getElementById("integralGainValue");
const derivativeGainValue = document.getElementById("derivativeGainValue");
const setAngleValue = document.getElementById("setAngleValue");
const sendDataButton = document.getElementById("sendDataButton");

//For Values of Rise Time, Settling Time, Steady-state error, and overshoot
const riseTimeValue = document.getElementById("riseTimeValue");
const settlingTimeValue = document.getElementById("settlingTimeValue");
const steadyStateErrorValue = document.getElementById("steadyStateErrorValue");
const overshootValue = document.getElementById("overshootValue");

//testing the functionality of button and sending data
function sendData() {
  console.log("Kp: " + proportionalGainValue.value);
  console.log("Ki: " + integralGainValue.value);
  console.log("Kd: " + derivativeGainValue.value);
  console.log("Angle: " + setAngleValue.value);
  console.log(riseTimeValue.textContent);
  console.log(settlingTimeValue.textContent);
  console.log(steadyStateErrorValue.textContent);
  console.log(overshootValue.textContent);
  console.log(deviceName.textContent);
}

//initiates the sending of data from website to ESP32
sendDataButton.addEventListener("click", sendData);
