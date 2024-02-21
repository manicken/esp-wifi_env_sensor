

window.addEventListener('load', setup);

var functionNames = [];
var shortDows = [];

var modal;
// Function to open the modal
function openModal(scheduleItem) {
  modal.style.display = 'block';
  document.querySelector("#modal-innerContent").innerHTML = scheduleItem.innerHTML;
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

function setup() {
  // Get the modal
  modal = document.getElementById('myModal');
  //openModal();
  let isMobileDevice = checkIfMobileDevice();
  
  //document.querySelector(".modal-content").style.maxHeight = document.body.clientHeight;
  if (isMobileDevice) {
    document.body.style.width = "auto";
    document.body.style.margin = "2 auto"; /* Center-align body content */
    document.querySelector(".modal-content").style.maxWidth = "95%";
    setState("You are using a Mobile Device");
  } else {
      document.body.style.width = "360px";
      document.querySelector(".modal-content").style.maxWidth = document.body.style.width;
      setState("You are using Desktop");
  }
  

  getFile("/schedule/getFunctionNames", function(contents1) {
    console.log(contents1);
    functionNames = JSON.parse(contents1);
    console.log("functions:");
    console.log(functionNames);
    getFile("/schedule/getShortDows", function(contents2) {
      console.log(contents2);
      shortDows = JSON.parse(contents2);
    
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

function getItemHtml(data) {
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
  else if (mode == 'explicit') mode = 'once';
  let params = data.params?data.params:"";
  let html = '<div class="schedule-item">';
  html += `<div class="schedule-item-var time" data-h="${data.h}" data-m="${data.m}" data-s="${data.s}">${time}</div>`;
  html += `<div class="schedule-item-var enable_switch"> <label class="el-switch"> <input type="checkbox" name="switch" ${enabled}> <span class="el-switch-style"></span> </label> </div>`
  html += `<div class="schedule-item-var details" data-mode="${data.mode}" data-func="${data.func}" data-params="${params}">`+
          `<p class="schedule-item-mode">${mode}</p>`+
          `<p class="schedule-item-func">${data.func}</p>`+
          `<p class="schedule-item-params">${params}</p>`+
          '</div>';
  html += '</div>';
  return html;
}

function drawItemList(data) {
  let html = "";
  for (let i=0;i<data.length;i++) {
    if (data[i].params == undefined) data[i].params = "";
    else data[i].params = JSON.stringify(data[i].params); // this needs to be edited as a string

    html += getItemHtml(data[i]);
    if (i < data.length-1)
      html += '<div class="verticalSeperator"></div>';
  }
  document.getElementById("item_list").innerHTML = html;

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
              openModal(event.currentTarget);
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