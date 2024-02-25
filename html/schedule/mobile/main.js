

window.addEventListener('load', setup);

var modal;
var scheduleItemTemplate;
// Function to open the modal
function openModal(scheduleItem) {
  modal.style.display = 'block';
  
  let modalBounding = document.getElementById("modal-content").getBoundingClientRect();
  console.log(modalBounding);
  let lastItem = document.getElementById("edit-params").getBoundingClientRect();
  console.log(lastItem);

  let newHeight =  modalBounding.height - lastItem.y;
  console.log("newHeight:" + newHeight);
  document.getElementById("edit-params").style.height = newHeight + "px";
  document.getElementById("edit-mode").value = scheduleItem.mode;
  refreshEditScheduleItem_mode(scheduleItem.mode);
  document.getElementById("edit-func-options").value = scheduleItem.func;
  document.getElementById("edit-params").value = scheduleItem.params;
  document.getElementById("edit-time-h").value = scheduleItem.h;
  document.getElementById("edit-time-m").value = scheduleItem.m;
  document.getElementById("edit-time-s").value = scheduleItem.s;
  document.getElementById("edit-timer-h").value = scheduleItem.h;
  document.getElementById("edit-timer-m").value = scheduleItem.m;
  document.getElementById("edit-timer-s").value = scheduleItem.s;
  document.getElementById("edit-time-y").value = scheduleItem.y?scheduleItem.y:2024;
  document.getElementById("edit-time-M").value = scheduleItem.M?scheduleItem.M:1;
  document.getElementById("edit-time-d").value = scheduleItem.d?scheduleItem.d:1;
  if (scheduleItem.D != undefined) {
    let weeklySel = document.getElementById("edit-weekly-" + scheduleItem.D);
    if (weeklySel != undefined)
      weeklySel.checked = true;
  }
    
  //document.querySelector("#modal-innerContent").innerHTML = scheduleItem.innerHTML;
}

function refreshEditScheduleItem_mode(mode) {
  if (mode == 'timer') {
    document.querySelector(".edit-time").classList.remove('active');
    document.querySelector(".edit-timer").classList.add('active');
    document.querySelector(".edit-weekly").classList.remove('active');
    document.querySelector(".edit-ymd").classList.remove('active');
  }
  else if (mode == 'daily') {
    document.querySelector(".edit-timer").classList.remove('active');
    document.querySelector(".edit-time").classList.add('active');
    document.querySelector(".edit-weekly").classList.remove('active');
    document.querySelector(".edit-ymd").classList.remove('active');
  }
  else if (mode == 'weekly') {
    document.querySelector(".edit-timer").classList.remove('active');
    document.querySelector(".edit-time").classList.add('active');
    document.querySelector(".edit-weekly").classList.add('active');
    document.querySelector(".edit-ymd").classList.remove('active');
  }
  else if (mode == 'explicit') {
    document.querySelector(".edit-timer").classList.remove('active');
    document.querySelector(".edit-time").classList.add('active');
    document.querySelector(".edit-weekly").classList.remove('active');
    document.querySelector(".edit-ymd").classList.add('active');
  }
}

// Function to close the modal
function closeModal() {
  modal.style.display = 'none';
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

function scheduleModeChanged(mode) 
{
  console.log(mode);
  refreshEditScheduleItem_mode(mode);
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
    let items = JSON.parse(itemsJsonStr);
    const functionNames = Object.keys(items);
    
    console.log("functions:");
    console.log(items, functionNames);
    
    document.getElementById("edit-func-options").innerHTML = getOptionsHtml(functionNames, true, functionNames[0]);
    
    getFile("/schedule/getShortDows", function(contents2) {
      console.log(contents2);
      let shortDows = JSON.parse(contents2);
    
      loadSchedules();
    }, function () { setState("error cannot load short DOW list"); });
  }, function() { setState("error cannot load function list"); });
  //setupWebSocket();
}

function fixTimeToHHMMSS(data) {
  data.h = data.h?( (data.h<10)?("0"+data.h):data.h ):"00";
  data.m = data.m?( (data.m<10)?("0"+data.m):data.m ):"00";
  data.s = data.s?( (data.s<10)?("0"+data.s):data.s ):"00";
}
function fixTimerValue(data) {
  data.h = data.h?data.h:0;
  data.m = data.m?data.m:0;
  data.s = data.s?data.s:0;
}

function setItemData(item, data) {
  console.log(data);
  let mode = data.mode;

  if (mode != 'timer') {
    fixTimeToHHMMSS(data);
    var time = data.h + ":" + data.m + ((data.s>0)?(":" + data.s):'');
  }
  else {
    fixTimerValue(data);
    let h = data.h, m = data.m, s = data.s;
    var time = ((h>0)?(h + " h"):'') + ((m>0)?(((h>0)?", ":'') + m + " m"):'') + ((s>0)?(((h>0 || m>0)?", ":'')  + s + " s"):'');
    //var time = ((h>0)?(h + " hours"):'') + ((m>0)?(((h>0)?", ":'') + m + " mins"):'') + ((s>0)?(((h>0 || m>0)?", ":'')  + s + " sec"):'');
  }
  let enabled = data.disabled==undefined ? 'checked':'';
  
  if (mode == 'daily') mode = 'Everyday';
  else if (mode == 'timer') mode = 'timer';
  else if (mode == 'weekly') mode = data.D;
  else if (mode == 'explicit') mode = data.y+"-"+data.M+"-"+data.d;
  let params = data.params?data.params:"";

  let timeItem = item.querySelector('.time');
  item.data = data;

  timeItem.innerHTML = time;
  item.querySelector('input').checked = enabled;
  item.querySelector('.schedule-item-mode').innerHTML = mode;
  item.querySelector('.schedule-item-func').innerHTML = data.func;
  item.querySelector('.schedule-item-params').innerHTML = params;
}

function drawItemList(data) {
  let item_list = document.getElementById("item_list");
  item_list.innerHTML = "";
  for (let i=0;i<data.length;i++) {
    if (data[i].params == undefined) data[i].params = "";
    else data[i].params = JSON.stringify(data[i].params); // this needs to be edited as a string
    let newItem = scheduleItemTemplate.cloneNode(true);
    newItem.removeAttribute('id');
    setItemData(newItem, data[i]);
    item_list.appendChild(newItem);
    newItem.children = "";

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
              openModal(event.currentTarget.data);
          }
          else
            setState("");
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

}