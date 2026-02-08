// SCRIPT FOR AUTO-TUNING (FIXED TUNING)
const socket = io();

// --- YOUR FIXED TUNED VALUES ---
const FIXED_KP = 15.0; // Change these to your ideal values
const FIXED_KI = 0.5;
const FIXED_KD = 3.5;

const sendDataButton = document.getElementById("sendDataButton");

// Elements for performance metrics
const riseTimeValue = document.getElementById("riseTimeValue");
const settlingTimeValue = document.getElementById("settlingTimeValue");
const steadyStateErrorValue = document.getElementById("steadyStateErrorValue");
const overshootValue = document.getElementById("overshootValue");

// Display the fixed values on the UI so the user knows what's running
document.getElementById("currentKp").innerText = FIXED_KP;
document.getElementById("currentKi").innerText = FIXED_KI;
document.getElementById("currentKd").innerText = FIXED_KD;

// --- Graph Setup (Same as Manual) ---
const ctx = document.getElementById("pidChart").getContext("2d");
const maxDataPoints = 50;
const pidChart = new Chart(ctx, {
  type: "line",
  data: {
    labels: [],
    datasets: [
      {
        label: "Actual Value",
        data: [],
        borderColor: "rgba(75, 192, 192, 1)",
        borderWidth: 2,
        tension: 0.3,
      },
      {
        label: "Setpoint",
        data: [],
        borderColor: "rgba(255, 99, 132, 1)",
        borderDash: [5, 5],
        borderWidth: 2,
        fill: false,
      },
    ],
  },
  options: {
    responsive: true,
    maintainAspectRatio: false,
    scales: { x: { display: false } },
  },
});

// --- Listen for Real-time Data ---
socket.on("realtimeData", (data) => {
  if (data.riseTime !== undefined)
    riseTimeValue.innerText = Number(data.riseTime).toFixed(2);
  if (data.settlingTime !== undefined)
    settlingTimeValue.innerText = Number(data.settlingTime).toFixed(2);
  if (data.steadyStateError !== undefined)
    steadyStateErrorValue.innerText = Number(data.steadyStateError).toFixed(2);
  if (data.overshoot !== undefined)
    overshootValue.innerText = Number(data.overshoot).toFixed(2);

  // Update Chart
  const now = new Date().toLocaleTimeString();
  pidChart.data.labels.push(now);
  pidChart.data.datasets[0].data.push(data.actualValue);
  pidChart.data.datasets[1].data.push(data.setpoint);

  if (pidChart.data.labels.length > maxDataPoints) {
    pidChart.data.labels.shift();
    pidChart.data.datasets.forEach((d) => d.data.shift());
  }
  pidChart.update("none");
});

// --- Send FIXED Values + User Setpoint ---
async function sendFixedTuning() {
  const setAngleValue = document.getElementById("setAngleValue").value;

  try {
    const response = await fetch("/api/command", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        kp: FIXED_KP,
        ki: FIXED_KI,
        kd: FIXED_KD,
        set: setAngleValue,
      }),
    });
    const result = await response.json();
    console.log("Fixed tuning and setpoint sent:", result.status);
  } catch (error) {
    console.error("Error sending auto command:", error);
  }
}

socket.on("clearGraph", () => {
  pidChart.data.labels = [];
  pidChart.data.datasets.forEach((d) => (d.data = []));
  pidChart.update();
});

sendDataButton.addEventListener("click", sendFixedTuning);
