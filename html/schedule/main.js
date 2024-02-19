let exampleList = [
  {"mode":"timer","m":10,"func":"sendEnvData"},
  {"mode":"daily","h":0,"m":0,"s":0,"func":"fan","params":{"val":255}},
  {"mode":"daily","h":8,"m":0,"s":0,"func":"fan","params":{"val":160}},
  {"mode":"daily","h":18,"m":0,"s":0,"func":"fan","params":{"val":32}}
];

window.addEventListener('load', setup);

var currentTab = "dataTable_timers";

var functionNames = [];
var shortDows = [];

function setup() {
/*
  document.getElementById('uploadForm').addEventListener('submit', function(event) {
    event.preventDefault(); // Prevent default form submission
    
    // Get file input element
    const fileInput = document.getElementById('fileInput');
    console.log(fileInput.files[0]);
    // Create FormData object
    const formData = new FormData();
    
    // Append file to FormData object
    formData.append('file', fileInput.files[0]);
    
    // Send AJAX request to server
    const xhr = new XMLHttpRequest();
    xhr.open('POST', '/edit');
    xhr.onreadystatechange = function() {
      if (xhr.readyState === XMLHttpRequest.DONE) {
        if (xhr.status === 200) {
          console.log('File uploaded successfully!');
        } else {
          console.error('Error uploading file:', xhr.statusText);
        }
      }
    };
    xhr.send(formData);
  }); */

  getFile("schedule/getFunctionNames", function(contents1) {
    console.log(contents1);
    functionNames = JSON.parse(contents1);
    console.log("functions:");
    console.log(functionNames);
    getFile("schedule/getShortDows", function(contents2) {
      console.log(contents2);
      shortDows = JSON.parse(contents2);
    
      loadConfiguration();
    }, function () { setState("error cannot load short DOW list"); });
  }, function() { setState("error cannot load function list"); });
  //setupWebSocket();
}

function SpecialJsonStringify(obj) {
  return JSON.stringify(obj, (key, value) => {
    if (key == "params") {
      if (value != "")
        return JSON.parse(value);
      else
        return undefined;
    }
    else
      return value;
  });
}

function getTableRowData_YMD(data) {
  let html = "";
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxYear center-text', data.y?data.y:2024) +'</td>';
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.M?data.M:1) +'</td>';
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.d?data.d:1) +'</td>';
  return html;
}

function getTableRowData_HMS(data) {
  let html = "";
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.h?data.h:0) +'</td>';
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.m?data.m:0) +'</td>';
  html += '<td class="centered-cell">'+ getTextInputHtml('', 'txtBoxHMS center-text', data.s?data.s:0) +'</td>';
  return html;
}

function getTableRowData_Func_Params_RemoveBtn(data) {
  let html = "";
  html += '<td class="centered-cell">'+ getSelectInputHtml('', 'selectBox center-text', functionNames, data.func) +'</td>';
  html += '<td>'+ getTextInputHtml('', 'txtBoxParams', data.params) + '</td>';
  html += '<td>' + getRemoveButtonHtml() + '</td>';
  return html;
}

function getTableRow_timers_or_daily(data) {
  let html = '<tr>';
  html += getTableRowData_HMS(data);
  html += getTableRowData_Func_Params_RemoveBtn(data);
  html += '</tr>';
  return html;
}

function getTableRow_weekly(data) {
  let html = '<tr>';
  html += '<td class="centered-cell">'+ getSelectInputHtml('', 'txtBoxDOW center-text', shortDows, data.D?data.D:shortDows[0]) +'</td>';
  html += getTableRowData_HMS(data);
  html += getTableRowData_Func_Params_RemoveBtn(data);
  html += '</tr>';
  return html;
}

function getTableRow_explicit(data) {
  let html = '<tr>';
  html += getTableRowData_YMD(data);
  html += getTableRowData_HMS(data);
  html += getTableRowData_Func_Params_RemoveBtn(data);
  html += '</tr>';
  return html;
}

function drawTables(data) {
  let html_dt_timers = "";
  let html_dt_daily = "";
  let html_dt_weekly = "";
  let html_dt_explicit = "";
  
  for (let i=0;i<data.length;i++)
  {
    if (data[i].params == undefined) data[i].params = "";
    else data[i].params = JSON.stringify(data[i].params); // this needs to be edited as a string

    if (data[i].mode == "timer")
      html_dt_timers += getTableRow_timers_or_daily(data[i]);
    else if (data[i].mode == "daily")
      html_dt_daily += getTableRow_timers_or_daily(data[i]);
    else if (data[i].mode == "weekly")
      html_dt_weekly += getTableRow_weekly(data[i]);
    else if (data[i].mode == "explicit")
      html_dt_explicit += getTableRow_explicit(data[i]);
  }
  
  document.getElementById("dataTableBody_timers").innerHTML = html_dt_timers;
  document.getElementById("dataTableBody_daily").innerHTML = html_dt_daily;
  document.getElementById("dataTableBody_weekly").innerHTML = html_dt_weekly;
  document.getElementById("dataTableBody_explicit").innerHTML = html_dt_explicit;
}

function redrawCurrentTab_dataTable() {
  if (currentTab == "dataTable_timers") drawTable_timers();
  else if (currentTab == "dataTable_daily") drawTable_daily();
  else if (currentTab == "dataTable_weekly") drawTable_weekly();
  else if (currentTab == "dataTable_explicit") drawTable_explicit();
}

function addNewItem()
{
  if (currentTab == "dataTable_timers") {
    let html = getTableRow_timers_or_daily({m:0,h:0,s:0,func:functionNames[0],params:""});
    document.getElementById("dataTableBody_timers").innerHTML += html;
  }
  else if (currentTab == "dataTable_daily") {
    let html = getTableRow_timers_or_daily({m:0,h:0,s:0,func:functionNames[0],params:""});
    document.getElementById("dataTableBody_daily").innerHTML += html;
  }
  else if (currentTab == "dataTable_weekly") {
    let html = getTableRow_weekly({D:shortDows[0],m:0,h:0,s:0,func:functionNames[0],params:""});
    document.getElementById("dataTableBody_weekly").innerHTML += html;
  }
  else if (currentTab == "dataTable_explicit") {
    let html = getTableRow_explicit({y:2024,M:1,d:1,m:0,h:0,s:0,func:functionNames[0],params:""});
    document.getElementById("dataTableBody_explicit").innerHTML += html;
  }
}

function removeItem(element, index, dtName)
{
  console.log(element.parentNode.parentNode);
  
  if (currentTab == "dataTable_timers") {
    var confirmed = confirm("Are you sure you want to remove the item:\n" + SpecialJsonStringify(getTimersOrDailyData(element.parentNode.parentNode)));
    if (confirmed == false) return;
    element.parentNode.parentNode.remove();
  }
  else if (currentTab == "dataTable_daily") {
    var confirmed = confirm("Are you sure you want to remove the item:\n" + SpecialJsonStringify(getTimersOrDailyData(element.parentNode.parentNode)));
    if (confirmed == false) return;
    element.parentNode.parentNode.remove();
  }
  else if (currentTab == "dataTable_weekly") {
    var confirmed = confirm("Are you sure you want to remove the item:\n" + SpecialJsonStringify(getWeeklyData(element.parentNode.parentNode)));
    if (confirmed == false) return;
    element.parentNode.parentNode.remove();
  }
  else if (currentTab == "dataTable_explicit") {
    var confirmed = confirm("Are you sure you want to remove the item:\n" + SpecialJsonStringify(getExplicitData(element.parentNode.parentNode)));
    if (confirmed == false) return;
    element.parentNode.parentNode.remove();
  }
}

function loadConfiguration()
{
  getFile("schedule/list.json", 
    function(contents){
      console.log(contents);
      let data = JSON.parse(contents);
      drawTables(data);
    },
    function() { setState("error cannot load schedule/list.json"); });
}

function getTimersOrDailyData(dataRow, mode) {
  let cells = dataRow.getElementsByTagName("td");
  return {
    mode:mode,
    h:parseInt(cells[0].children[0].value),
    m:parseInt(cells[1].children[0].value),
    s:parseInt(cells[2].children[0].value),
    func:cells[3].children[0].value,
    params:cells[4].children[0].value
  };
}

function getWeeklyData(dataRow) {
  console.log(dataRow);
  let cells = dataRow.getElementsByTagName("td");
  return {
    mode:"weekly",
    D:cells[0].children[0].value,
    h:parseInt(cells[1].children[0].value),
    m:parseInt(cells[2].children[0].value),
    s:parseInt(cells[3].children[0].value),
    func:cells[4].children[0].value,
    params:cells[5].children[0].value
  };
}

function getExplicitData(dataRow) {
  let cells = dataRow.getElementsByTagName("td");
  return {
    mode:"explicit",
    y:parseInt(cells[0].children[0].value),
    M:parseInt(cells[1].children[0].value),
    d:parseInt(cells[2].children[0].value),
    h:parseInt(cells[3].children[0].value),
    m:parseInt(cells[4].children[0].value),
    s:parseInt(cells[5].children[0].value),
    func:cells[6].children[0].value,
    params:cells[7].children[0].value
  };
}

function saveConfiguration()
{
  let data = [];
  let rows = document.getElementById("dataTableBody_timers").getElementsByTagName("tr");
  for (let i=0;i<rows.length;i++) {
    let newItem = getTimersOrDailyData(rows[i], "timer");
    
    if(newItem.h == 0) newItem.h = undefined;
    if(newItem.m == 0) newItem.m = undefined;
    if(newItem.s == 0) newItem.s = undefined;
    
    if (newItem.h == undefined && newItem.m == undefined && newItem.s == undefined) newItem.s = 1; // at least one second
    
    data.push(newItem);
  }
  
  rows = document.getElementById("dataTableBody_daily").getElementsByTagName("tr");
  for (let i=0;i<rows.length;i++) {
    let newItem = getTimersOrDailyData(rows[i], "daily");
    
    // can skip both minutes and second if they are zero, hour is allways specified
    if(newItem.m == 0) newItem.m = undefined;
    if(newItem.s == 0) newItem.s = undefined;
    data.push(newItem);
  }
  
  rows = document.getElementById("dataTableBody_weekly").getElementsByTagName("tr");
  for (let i=0;i<rows.length;i++) {
    let newItem = getWeeklyData(rows[i]);

    // can skip both minutes and second if they are zero, hour is allways specified
    if(newItem.m == 0) newItem.m = undefined;
    if(newItem.s == 0) newItem.s = undefined;
    data.push(newItem);
  }
  
  rows = document.getElementById("dataTableBody_explicit").getElementsByTagName("tr");
  for (let i=0;i<rows.length;i++) {
    let newItem = getExplicitData(rows[i]);
    
    // can skip both minutes and second if they are zero, hour is allways specified
    if(newItem.m == 0) newItem.m = undefined;
    if(newItem.s == 0) newItem.s = undefined;
    data.push(newItem);
  }

  var dataJSON = "[\n";
  for (let i=0;i<data.length;i++) {
    dataJSON += "  " + SpecialJsonStringify(data[i])
    if (i<(data.length-1)) dataJSON += ',';
    dataJSON += "\n";
  }
  dataJSON += "]";
  console.log(dataJSON);
  
  
  postFile("schedule/list.json", dataJSON, "application/json", newScheduleFile_Posted, newScheduleFile_NotPosted)
}

function newScheduleFile_Posted(){
  setState("file saved");
  getFile("/schedule/refresh", function(msg) { setState(msg);}, function() {setState("fail to load new schedule");});
}

function newScheduleFile_NotPosted() {
  setState("fail to save file");
}

function openTab(evt, tabName) {
  var i, tabContent, tabLinks;
  currentTab = tabName;

  tabContent = document.getElementsByClassName("tab-content");
  for (i = 0; i < tabContent.length; i++) {
    tabContent[i].style.display = "none";
  }

  tabLinks = document.getElementsByClassName("tab");
  for (i = 0; i < tabLinks.length; i++) {
    tabLinks[i].className = tabLinks[i].className.replace(" active", "");
  }

  document.getElementById(tabName).style.display = "block";
  evt.currentTarget.className += " active";
}

// **********  get common html ************

function getRemoveButtonHtml() {
  return `<button type="button" onclick="removeItem(this)" style="width:30px; height:30px; font-weight:bold;">X</button>`;
}

function getTextInputHtml(id, className, value) {
  if (id != "") id = `id="${id}" `;
  return `<input ${id}class="${className}" type="text" value='${value}'>`;
}

function getSelectInputHtml(id, selectClassName, options, value) {
  if (id != "") id = `id="${id}" `;
  
  let optionsHtml = getOptionsHtml(options, true, value);
  return `<select ${id}class="${selectClassName}">${optionsHtml}</select>`; 
}

function getOptionsHtml(options, useOptionsAsValues, value) {
  let html = "";
  for (let i=0;i<options.length;i++)
  {
    let selected=(options[i]==value)?'selected="true"':"";
    
    if (useOptionsAsValues == true)
      html += `<option ${selected} value="${options[i]}">${options[i]}</option>`;
    else
      html += `<option ${selected} value="${i}">${options[i]}</option>`;
  }
  return html;
}

// **********   common helpers ************

var webSocketConnection;

function setupWebSocket()
{
  console.log(location.hostname);
  webSocketConnection = new WebSocket('ws://' + location.hostname + ':81/', ['arduino']);
  webSocketConnection.onopen = function () {
    webSocketConnection.send('Connect ' + new Date());
  };
  webSocketConnection.onerror = function (error) {
    console.log('WebSocket Error ', error);
  };
  webSocketConnection.onmessage = function (e) {
    console.log('Debug:\n'+ e.data);
  };
  webSocketConnection.onclose = function () {
    console.log('WebSocket connection closed');
  };
}

function setState(msg) {
  document.getElementById("info").innerHTML = msg;
}

function getFile(path, whenLoaded, onError) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (xhttp.readyState == 4) {
      if (xhttp.status == 200) {
        if (whenLoaded != undefined) whenLoaded(xhttp.responseText);
      }
      else // surely a not found error
      {
        if (onError != undefined) onError();
      }
    }
  };
  xhttp.open('GET', path, true);
  xhttp.send();
}

function postFile(path, data, type, onOK, onError){
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onload = function() {
    if (xmlHttp.status != 200) { 
      if (onError != undefined) onError();
      else console.log("post file fail:" + path);
    }
    else {
      if (onOK != undefined) onOK();
      else console.log("post file ok:" + path);
    }
  };
  var formData = new FormData();
  formData.append("data", new Blob([data], { type: type }), "\\\\"+path);
  //formData.append("file", data);//, path);
  //formData.append("data", data);
  //formData.append("type", type);
  //formData.append("path", path);
  console.log(path);
  
  xmlHttp.open("POST", "/edit");
  xmlHttp.send(formData);
  
}


