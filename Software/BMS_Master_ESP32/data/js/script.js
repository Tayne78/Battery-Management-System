function updateMeasuredValues() {
  return new Promise((resolve, reject) => {
    fetch("/api/measured-values")
      .then(async response => {
        if (!response.ok) {
          throw new Error("Network response was not ok");
        }
        const body = await response.json();
        console.log(body);
        return Promise.resolve(body);
      })
      .then(data => {
        // Update the measured values on your webpage using JavaScript
        resolve(data);
        // Repeat for other cells
      })
      .catch(error => {
        console.error("Fetch error:", error);
        reject();
      });
  });

}

const renderTable = (number_of_slaves) => {
  const $table = $('#batteryDataTableBody');
  for (let i = 0; i < number_of_slaves; ++i) {
    const $tableRow = $('<tr>').attr({ id: 'cell' + (i + 1) });
    const $tableHead = $('<th>').attr({ scope: 'row' }).text(i + 1);
    const $voltage = $('<td>').attr({ id: 'voltage' + (i + 1) });
    const $temperature = $('<td>').attr({ id: 'temperature' + (i + 1) });
    const $status = $('<td>').attr({ id: 'status' + (i + 1) });

    $tableRow
      .append($tableHead)
      .append($voltage)
      .append($temperature)
      .append($status);
    $table.append($tableRow);
  }
}

const updateTable = data => {
  for (let i = 1; i <= data.NUMBER_OF_SLAVES; i++) {
    $("#voltage" + i).text(data["voltage" + i]);
    $("#temperature" + i).text(data["temperature" + i]);
    if(data["status"+i] == 1){
    $("#status" + i).text("Balancing").addClass("red");
    }
    else{
      $("#status" + i).text("Idle").addClass("green");
      }
  }
  $("⁠#averageTemp").text(data["averageTemp"]+"°C");
  $("#overallVolt").text(((parseInt(data["overallVolt"])/1000).toFixed(2))+"V");
  $("#maxCellVoltage").text(data["maxCellVoltage"]+"mV");
  $("#maxCell").text("Z"+data["maxCell"]);
  $("#minCellVoltage").text(data["minCellVoltage"]+"mV");
  $("#minCell").text("Z"+data["minCell"]);
  $("#differenceMaxMin").text(data["differenceMaxMin"]+"mV");
  $("#MaxMinCells").text("Z"+data["maxCell"] + " -" + " Z"+data["minCell"]);
  $("#numCells").text(data["NUMBER_OF_SLAVES"]);
  $("#akkutyp").text(data["akkutyp"]);

  // setColorBasedOnValue(data.voltage1, "voltage1");
  // setColorBasedOnValue(data.temperature1, "temperature1");
}

$(() => {

  const update = () => {
    updateMeasuredValues().then(data => {
      updateTable(data);
    });
  }

  $("#proceedBtn").click(function(){
    sendSelectedBatteryType();

  });
  $("#sendSlaves").click(function(){
    if(parseInt($("#slaveCountInput").val())>255)
    {
      alert("Error! Zu viele Slaves wurden ausgewählt. Es dürfen maximal 255 Slaves ausgewählt werden.")
    }
    else if(parseInt($("#slaveCountInput").val()) <= 0)
    {
      alert("Error! Ungültige Eingabe")
    }
    else{
    sendNumberOfSlaves();
    alert("Anzahl der Slaves wurden konfiguriert. Warten Sie bis die Tabelle befüllt wurde. (<10s)");
    }
  });
  updateMeasuredValues().then(data => {
    console.log("UPDATE")
    renderTable(data.NUMBER_OF_SLAVES);
    updateTable(data);
    setInterval(update, 1000);
  }).catch(err => {
    console.log(err);
    $('body').append($('<p>').text("Error"));
  });
})

function sendSelectedBatteryType() {
  var selectedBattery = document.getElementById("batterySelect").value;
  console.log("sendSelectedBAtteryType()");

  var url = '/api/akkutyp'; 

  var data = { batteryType: selectedBattery };

  // POST-Anfrage konfigurieren
  var options = {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(data)
  };

  // POST-Anfrage senden
  fetch(url, options)
    .then(response => {
      if (response.ok) {
        console.log('POST-Anfrage erfolgreich gesendet!');
      } else {
        console.error('Fehler beim Senden der POST-Anfrage:', response.statusText);
      }
    })
    .catch(error => console.error('Fehler beim Senden der POST-Anfrage:', error));
}

function sendNumberOfSlaves() {

  var slaveCount = document.getElementById('slaveCountInput').value;

    

    const $table = $('#batteryDataTableBody');
    $("#batteryDataTableBody tr").detach();
    for (let i = 0; i < slaveCount; ++i) {
      const $tableRow = $('<tr>').attr({ id: 'cell' + (i + 1) });
      const $tableHead = $('<th>').attr({ scope: 'row' }).text(i + 1);
      const $voltage = $('<td>').attr({ id: 'voltage' + (i + 1) });
      const $temperature = $('<td>').attr({ id: 'temperature' + (i + 1) });
      const $status = $('<td>').attr({ id: 'status' + (i + 1) });
  
      $tableRow
        .append($tableHead)
        .append($voltage)
        .append($temperature)
        .append($status);
      $table.append($tableRow);
    }

  console.log("sendNumberOfSlaves");

  var url = '/api/numOfSlaves'; 

  var data = { numOfSlaves: slaveCount };

  // POST-Anfrage konfigurieren
  var options = {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify(data)
  };

  // POST-Anfrage senden
  fetch(url, options)
    .then(response => {
      if (response.ok) {
        console.log('POST-Anfrage erfolgreich gesendet!');
      } else {
        console.error('Fehler beim Senden der POST-Anfrage:', response.statusText);
      }
    })
    .catch(error => console.error('Fehler beim Senden der POST-Anfrage:', error));
}