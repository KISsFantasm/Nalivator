
let inputs;

window.onload = function() {
    inputs = { 
        wIP:{
            0: document.getElementById("wIP0"),
            1: document.getElementById("wIP1"),
            2: document.getElementById("wIP2"),
            3: document.getElementById("wIP3")
        },
        wGate:{
            0: document.getElementById("wGate0"),
            1: document.getElementById("wGate1"),
            2: document.getElementById("wGate2"),
            3: document.getElementById("wGate3")
        },
        wMask:{
            0: document.getElementById("wMask0"),
            1: document.getElementById("wMask1"),
            2: document.getElementById("wMask2"),
            3: document.getElementById("wMask3")
        },
        apIP:{
            0: document.getElementById("apIP0"),
            1: document.getElementById("apIP1"),
            2: document.getElementById("apIP2"),
            3: document.getElementById("apIP3")
        },
        apGate:{
            0: document.getElementById("apGate0"),
            1: document.getElementById("apGate1"),
            2: document.getElementById("apGate2"),
            3: document.getElementById("apGate3")
        },
        apMask:{
            0: document.getElementById("apMask0"),
            1: document.getElementById("apMask1"),
            2: document.getElementById("apMask2"),
            3: document.getElementById("apMask3")
        },
        remote:{
            0: document.getElementById("rIP0"),
            1: document.getElementById("rIP1"),
            2: document.getElementById("rIP2"),
            3: document.getElementById("rIP3")
        },
        checkAP: document.getElementById("checkAP"),
        wifiSsid: document.getElementById("wifiSsid"),
        wifiPass: document.getElementById("wifiPass"),
        apSsid: document.getElementById("apSsid"),
        apPass: document.getElementById("apPass"),
    }
    load();
}

function logout(){
    window.location.assign("/LogIn");
}

function load(){
    let uri = "/settingLoad";
    let xhr = new XMLHttpRequest();
    xhr.open("POST", uri, true);
    xhr.onload = function () {
        if (this.status == 200){
            parseResponse(this.response);
        }
    };
    xhr.send();
}

function parseResponse(resp){
    let str = resp.split("|");
    for(let idx in str){
        if (idx == 4) {inputs.checkAP.checked = parseInt(str[idx]); continue;}
        if (idx == 5) {inputs.wifiSsid.value = str[idx]; continue;}
        if (idx == 6) {inputs.wifiPass.value = str[idx]; continue;}
        if (idx == 7) {inputs.apSsid.value = str[idx]; continue;}
        if (idx == 8) {inputs.apPass.value = str[idx]; continue;}
        let val = str[idx].split("-");
        inputs.wIP[idx].value=val[0];
        inputs.wGate[idx].value=val[1];
        inputs.wMask[idx].value=val[2];
        inputs.apIP[idx].value=val[3];
        inputs.apGate[idx].value=val[4];
        inputs.apMask[idx].value=val[5];
        inputs.remote[idx].value=val[6];
    }
}

function save(){
    let str = "";
    for(idx in inputs){
        if (idx != "wIP" && idx != "wGate" && idx != "wMask" && idx != "apIP" && idx != "apMask" && idx != "apGate" && idx != "remote") continue;
        for(i in inputs[idx]){
            let val = parseInt(inputs[idx][i].value);
            if (!isValid(val,"short")) {alert("Некоректне значення адреси"); return;}
            str += val;
            if (i != 3) str += ".";
        }
        str += "|"; 
    }
    str += inputs.checkAP.checked+"|";
    if(isValid(inputs.wifiSsid.value, "text")) {str += inputs.wifiSsid.value+"|";} else {alert("WiFi Ssid містить неприпустимі символи"); return;};
    if(isValid(inputs.wifiPass.value, "text")) {str += inputs.wifiPass.value+"|";} else {alert("WiFi Password містить неприпустимі символи"); return;};
    if(isValid(inputs.apSsid.value, "text")) {str += inputs.apSsid.value+"|";} else {alert("AP Ssid містить неприпустимі символи"); return;};
    if(isValid(inputs.apPass.value, "text")) {str += inputs.apPass.value+"|";} else {alert("AP Password містить неприпустимі символи"); return;};    
    if(inputs.apPass.value.length < 8) {alert("AP Пароль має містити від 8 до 15 символів"); return};
    let uri = "/settingSave";
    uri += "?paramStr="+str;
    let xhr = new XMLHttpRequest();
    xhr.open("POST", uri, true);
    xhr.onload = function () {
    };
    xhr.send();
}

function isValid(val, type){
    switch(type){
        case "short": return (val >=0 && val <= 255);
        case "text": return (val.search(/[^\w\-]/g) == -1);
    }
}