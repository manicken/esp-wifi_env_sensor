
function checkIfMobileDevice() {
    /* Storing user's device details in a variable*/
    let details = navigator.userAgent;

    /* Regular expression containing some mobile devices keywords to search it in details string*/
    let regexp = /android|iphone|kindle|ipad/i;

    /* Test the regular expression against the details string, it returns boolean value*/
    let isMobileDevice = regexp.test(details);
    return isMobileDevice;
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
    return `<button type="button" class="removeButton" onclick="removeItem(this)">X</button>`;
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
    formData.append("data", new Blob([data], { type: type }), path);
    //formData.append("file", data);//, path);
    //formData.append("data", data);
    //formData.append("type", type);
    //formData.append("path", path);
    console.log(path);
    
    xmlHttp.open("POST", "/edit");
    xmlHttp.send(formData);
    
}
  