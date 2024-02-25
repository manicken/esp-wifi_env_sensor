

window.addEventListener('load', setup);
var functionDefs;

var modal;
var scheduleItemTemplate;
var currentEditItem;
// Function to open the modal
function openModal() {
  let data = currentEditItem.data;
  modal.style.display = 'block';
  
  let modalBounding = modal.querySelector("#modal-content").getBoundingClientRect();
  console.log(modalBounding);
  let lastItem = modal.querySelector("#edit-params").getBoundingClientRect();
  console.log(lastItem);

  let newHeight =  modalBounding.height - lastItem.y;
  console.log("newHeight:" + newHeight);
  modal.querySelector("#edit-params").style.height = newHeight + "px";
  modal.querySelector("#edit-mode").value = data.mode;
  refreshEditScheduleItem_mode(data.mode);
  modal.querySelector("#edit-func-options").value = data.func;
  modal.querySelector("#edit-params").value = data.params?JSON.stringify(data.params, null, 2):"";
  modal.querySelector("#edit-time-h").value = data.h?data.h:0;
  modal.querySelector("#edit-time-m").value = data.m?data.m:0;
  modal.querySelector("#edit-time-s").value = data.s?data.s:0;
  modal.querySelector("#edit-timer-h").value = data.h?data.h:0;
  modal.querySelector("#edit-timer-m").value = data.m?data.m:0;
  modal.querySelector("#edit-timer-s").value = data.s?data.s:0;
  modal.querySelector("#edit-time-y").value = data.y?data.y:2024;
  modal.querySelector("#edit-time-M").value = data.M?data.M:1;
  modal.querySelector("#edit-time-d").value = data.d?data.d:1;
  if (data.D != undefined) {
    let weeklySel = modal.querySelector("#edit-weekly-" + data.D);
    if (weeklySel != undefined)
      weeklySel.checked = true;
  }
    
  //document.querySelector("#modal-innerContent").innerHTML = scheduleItem.innerHTML;
}

function refreshEditScheduleItem_mode(mode) {
  if (mode == 'timer') {
    modal.querySelector(".edit-time").classList.remove('active');
    modal.querySelector(".edit-timer").classList.add('active');
    modal.querySelector(".edit-weekly").classList.remove('active');
    modal.querySelector(".edit-ymd").classList.remove('active');
  }
  else if (mode == 'daily') {
    modal.querySelector(".edit-timer").classList.remove('active');
    modal.querySelector(".edit-time").classList.add('active');
    modal.querySelector(".edit-weekly").classList.remove('active');
    modal.querySelector(".edit-ymd").classList.remove('active');
  }
  else if (mode == 'weekly') {
    modal.querySelector(".edit-timer").classList.remove('active');
    modal.querySelector(".edit-time").classList.add('active');
    modal.querySelector(".edit-weekly").classList.add('active');
    modal.querySelector(".edit-ymd").classList.remove('active');
  }
  else if (mode == 'explicit') {
    modal.querySelector(".edit-timer").classList.remove('active');
    modal.querySelector(".edit-time").classList.add('active');
    modal.querySelector(".edit-weekly").classList.remove('active');
    modal.querySelector(".edit-ymd").classList.add('active');
  }
}

// Function to close the modal
function closeModal() {
  modal.style.display = 'none';
}

function okModal() {
  let data = currentEditItem.data;
  
  data.mode = modal.querySelector("#edit-mode").value;
  data.func = modal.querySelector("#edit-func-options").value;
  data.params = JSON.parse(modal.querySelector("#edit-params").value);
  let mode = data.mode;
  if (mode == 'timer') {
    data.h = parseInt(modal.querySelector("#edit-timer-h").value);
    data.m = parseInt(modal.querySelector("#edit-timer-m").value);
    data.s = parseInt(modal.querySelector("#edit-timer-s").value);
  } 
  else if (mode == 'daily' || mode == 'weekly' || mode == 'explicit') { // inlcude all here instead of just using else for future implementations
    data.h = parseInt(modal.querySelector("#edit-time-h").value);
    data.m = parseInt(modal.querySelector("#edit-time-m").value);
    data.s = parseInt(modal.querySelector("#edit-time-s").value);
    if (mode == 'weekly') {
      data.D = getSingleDOWselection();
    }
    else if (mode == 'explicit') {
      data.y = parseInt(modal.querySelector("#edit-time-y").value);
      data.M = parseInt(modal.querySelector("#edit-time-M").value);
      data.d = parseInt(modal.querySelector("#edit-time-d").value);
    }
  }
  //currentEditItem.data = data;
  setItemElementsFromItemData(currentEditItem);
  console.log("ok pressed");
  modal.style.display = 'none';
}

function getSingleDOWselection() {
  var radios = document.querySelectorAll('.edit-weekly input[type="radio"]');
  for (let i=0;i<radios.length; i++) {
    if (radios[i].checked == true)
    {
      let id = radios[i].id;
      return id.substring(id.lastIndexOf('-') + 1);
    }
  }
  return "invalid";
}

function getAllDOWselections() {
  var radios = document.querySelectorAll('.edit-weekly.active input[type="radio"]');
  var checkedStates = {};

  radios.forEach(function(radio) {
      checkedStates[radio.id] = radio.checked;
  });

  return checkedStates;
}

// Close the modal when clicking outside of it
window.onclick = function(event) {
  if (event.target == modal) {
    closeModal();
  }
}

function handleNumberInputKeyDown(event) {
  // Get the key code of the pressed key
  var keyCode = event.keyCode || event.which;

  // Cancel the key press if it's not a number key (0-9)
  /*if (keyCode < 48 || keyCode > 57) {
      event.preventDefault(); // Cancel the default action of the key press
  }*/
}

function handleNumberInputChange(input) {
  let valueStr = input.value.replace(/[^\d.-]/g, '');
  if (valueStr == "") valueStr = input.min; // provide any number in case input.value only contains non digits
  // get current value and Remove non-integer characters
  var value = parseInt(valueStr);
  var max = parseInt(input.max);
  var min = parseInt(input.min);
  var leadingZeroAttribute = input.getAttribute('leadingZero');

  console.log(value + " " + max + " " + min);
  if (value > max) { value = max; }
  if (value < min) { value = min; }
  
  console.log(leadingZeroAttribute!=undefined);
  if (leadingZeroAttribute != undefined) {
    
    if (value < 10)
      value = "0" + value;
  }
  input.value = value;
  console.log(input.value);
}

function scheduleFuncChanged(funcName)
{
  //console.log(funcName);
  //console.log(functionDefs[funcName]);
  if (functionDefs[funcName] != '') {
    //console.log("active");
      modal.querySelector(".edit-params-class").classList.add('active');
  }
  else {
    //console.log("inactive");
      modal.querySelector(".edit-params-class").classList.remove('active');
  }
}

function setup() {
  // Get the modal
  modal = document.getElementById('myModal');
  //openModal();
  let isMobileDevice = checkIfMobileDevice();
  
  //document.querySelector(".modal-content").style.maxHeight = document.body.clientHeight;
  if (isMobileDevice) {
    document.body.style.width = "auto";
    document.body.style.margin = "2 auto"; /* Center-align body content */
    document.querySelector("#modal-content").style.width = "99%";
    document.querySelector("#modal-content").style.height = "99.5%";
    setState("You are using a Mobile Device");
  } else {
      document.body.style.width = "360px";
      console.log(document.body.clientHeight);
      document.querySelector("#modal-content").style.width = "370px";
      document.querySelector("#modal-content").style.height = "890px";
      setState("You are using Desktop");
  }
  // get a copy of the template
  scheduleItemTemplate = document.getElementById("schedule-item-template").cloneNode(true);

  getFile("/schedule/getFunctionNames", function(itemsJsonStr) {
    console.log(itemsJsonStr);
    functionDefs = JSON.parse(itemsJsonStr);
    const functionNames = Object.keys(functionDefs);
    
    console.log("functions:");
    console.log(functionDefs, functionNames);
    
    document.getElementById("edit-func-options").innerHTML = getOptionsHtml(functionNames, true, functionNames[0]);
    
    getFile("/schedule/getShortDows", function(contents2) {
      console.log(contents2);
      let shortDows = JSON.parse(contents2);
    
      loadSchedules();
    }, function () { setState("error cannot load short DOW list"); });
  }, function() { setState("error cannot load function list"); });
  //setupWebSocket();
}

function getFixedTimeToHHMMSS(data) {
  return { h: data.h?( (data.h<10)?("0"+data.h):data.h ):"00",
           m: data.m?( (data.m<10)?("0"+data.m):data.m ):"00",
           s: data.s?( (data.s<10)?("0"+data.s):data.s ):"00"};
}
function getFixedTimerValue(data) {
  return {h: data.h?data.h:0,
          m: data.m?data.m:0,
          s: data.s?data.s:0};
}

function setItemElementsFromItemData(item) {
  let data = item.data;
  console.log(data);
  let mode = data.mode;

  if (mode != 'timer') {
    let tv = getFixedTimeToHHMMSS(data);
    var compositeTime = tv.h + ":" + tv.m + ((tv.s>0)?(":" + tv.s):'');
  }
  else {
    let tv = getFixedTimerValue(data);
    
    var compositeTime = ((tv.h>0)?(tv.h + " h"):'') + ((tv.m>0)?(((tv.h>0)?", ":'') + tv.m + " m"):'') + ((tv.s>0)?(((tv.h>0 || tv.m>0)?", ":'')  + tv.s + " s"):'');
    //var time = ((h>0)?(h + " hours"):'') + ((m>0)?(((h>0)?", ":'') + m + " mins"):'') + ((s>0)?(((h>0 || m>0)?", ":'')  + s + " sec"):'');
  }
  let enabled = (data.disabled==undefined) ? true:false;
  
  if (mode == 'daily') mode = 'Everyday';
  else if (mode == 'timer') mode = 'timer';
  else if (mode == 'weekly') mode = data.D;
  else if (mode == 'explicit') mode = data.y+"-"+data.M+"-"+data.d;
  let params = data.params?JSON.stringify(data.params):"";

  let timeItem = item.querySelector('.time');
  item.data = data;

  timeItem.innerHTML = compositeTime;
  item.querySelector('input').checked = enabled;
  item.querySelector('.schedule-item-mode').innerHTML = mode;
  item.querySelector('.schedule-item-func').innerHTML = data.func;
  item.querySelector('.schedule-item-params').innerHTML = params;
}

function drawItemList(data) {
  let item_list = document.getElementById("item_list");
  item_list.innerHTML = "";
  for (let i=0;i<data.length;i++) {
    //if (data[i].params == undefined) data[i].params = {};
    //else data[i].params = JSON.stringify(data[i].params); // this needs to be edited as a string
    let newItem = scheduleItemTemplate.cloneNode(true);
    newItem.removeAttribute('id');
    newItem.data = data[i];
    setItemElementsFromItemData(newItem);
    item_list.appendChild(newItem);

    if (i < data.length-1) {
      let seperator = document.createElement("div");
      seperator.className = "verticalSeperator";
      item_list.appendChild(seperator);
    }
  }

  const scheduleItems = document.getElementsByClassName('schedule-item');
  console.log(scheduleItems);
  for (let i=0;i<scheduleItems.length;i++) {
      const checkbox = scheduleItems[i].querySelector('input');
      const checkbox2 = scheduleItems[i].querySelector('.el-switch-style');
      //console.log(scheduleItem);

      scheduleItems[i].addEventListener('click', function (event) {
        //console.log("checkbox:",checkbox);
        //console.log("target:",event.target);
          if (event.target !== checkbox && event.target !== checkbox2) {
              // Perform your onClick() event here
              //setState('Schedule item clicked!'+event.currentTarget.innerHTML);
              console.log(event.currentTarget.data);
              currentEditItem = event.currentTarget;
              openModal();
          }
          else if (event.target === checkbox) {
            //setState("");
            console.log("checkbox clicked:" + event.target.checked);
            if (event.target.checked) { delete event.currentTarget.data.disabled; }
            else event.currentTarget.data.disabled = true;
          }
            
      });
  }
}

function loadSchedules() {
  getFile("/schedule/list.json", 
    function(contents){
      console.log(contents);
      let data = JSON.parse(contents);
      drawItemList(data);
    },
    function() { setState("error cannot load schedule/list.json"); });
}

function saveSchedules() {
  let items = document.getElementsByClassName("schedule-item");
  var dataJSON = "[\n";
  for (let i=0;i<items.length;i++) {
    let item = items[i].data;
    let newItem = {mode:item.mode};
    
    if (item.mode == 'timer') {
      newItem.h = (item.h!=0)?item.h:undefined;
      newItem.m = (item.m!=0)?item.m:undefined;
      newItem.s = (item.s!=0)?item.s:undefined;
      if (newItem.h == undefined && newItem.m == undefined && newItem.s == undefined) newItem.s = 1; // at least one second
    }
    else if (item.mode == 'daily' || item.mode == 'weekly' || item.mode == 'explicit') {
      if (item.mode == 'weekly')
        newItem.D = item.D;
      else if (item.mode == 'explicit') {
        newItem.y = item.y;
        newItem.M = item.M;
        newItem.d = item.d;
      }
      newItem.h = item.h; // allways set hour for clarification
      newItem.m = (item.m!=0)?item.m:undefined;
      newItem.s = (item.s!=0)?item.s:undefined;
    }
    newItem.func = item.func;
    if (item.params != "") newItem.params = item.params;

    dataJSON += JSON.stringify(newItem);
    if (i<(items.length-1))
      dataJSON += ',';
    dataJSON += "\n";
    //console.log(newItem);
  }
  dataJSON += "]";

  console.log(dataJSON);
}