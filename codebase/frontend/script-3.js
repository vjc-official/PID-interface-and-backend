//SCRIPT FOR MANUAL PAGE

//For device name
const deviceName = document.getElementById("deviceName");

//For Values of Kp, Ki, Kd, Set Angle, and Button
const sendDataButton = document.getElementById("sendDataButton");

//For Values of Rise Time, Settling Time, Steady-state error, and overshoot
const riseTimeValue = document.getElementById("riseTimeValue");
const settlingTimeValue = document.getElementById("settlingTimeValue");
const steadyStateErrorValue = document.getElementById("steadyStateErrorValue");
const overshootValue = document.getElementById("overshootValue");

//testing the functionality of button and sending data
async function sendData() {
  const proportionalGainValue = document.getElementById("proportionalGainValue").value;
  const integralGainValue = document.getElementById("integralGainValue").value;
  const derivativeGainValue = document.getElementById("derivativeGainValue").value;
  const setAngleValue = document.getElementById("setAngleValue").value;
  console.log(proportionalGainValue);
  console.log(integralGainValue);
  console.log(derivativeGainValue);
  console.log(setAngleValue);
    try {
      const response = await fetch('/api/command', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ 
        kp : proportionalGainValue,
        ki : integralGainValue, 
        kd : derivativeGainValue, 
        set : setAngleValue })
    });       
  const result = await response.json();
  console.log(result.status);
  } 
  catch (error) {
    console.error('Error sending command:', error);
  }
}

//initiates the sending of data from website to ESP32
sendDataButton.addEventListener("click", sendData);
