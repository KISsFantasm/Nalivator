
let request = 0;
let step;
let maxNReq = 20;
let validCar = true;
let validSection = true;
let numParam = 8;
let first = true;
let chartData = {};
let chartRefresh = false;
let currentChart = {'car' : '', 'section' : '', 'date' : ''};

let mainW, settingW, recordW, pass, loadFrame, loadText, connectFrame, connectText;
let chartCar, chartSection, chartDate;
let loadWindow = [];
let connectWindow = [];

let accessLabel;
let b1, b2, b3, b4, doseB;
let selector, carN, carS;
let par0, par1, par2, par3, par4, par5, par6, par7;
let val0, val1, val2, val3;
let current, currentSt;

function initParams(){
    par0 = document.getElementById('param_0');
    par1 = document.getElementById('param_1');
    par2 = document.getElementById('param_2');
    par3 = document.getElementById('param_3');
    par4 = document.getElementById('param_4');
    par5 = document.getElementById('param_5');
    par6 = document.getElementById('param_6');
    par7 = document.getElementById('param_7');

    val0 = document.getElementById('val_0');
    val1 = document.getElementById('val_1');
    val2 = document.getElementById('val_2');
    val3 = document.getElementById('val_3'); 

    current = document.getElementById('current');
    currentSt = document.getElementById('status');
     
    sb1 = document.getElementById('sb1');
    sb2 = document.getElementById('sb2');
    sb3 = document.getElementById('sb3');
    sb4 = document.getElementById('sb4');
    doseB = document.getElementById('doseB');
    selector = document.getElementById('option');
    carN = document.getElementById('car_number');
    carS = document.getElementById('car_section');

    mainW = document.getElementById("mainW");
    settingW = document.getElementById("settingW");
    recordW = document.getElementById("recordW");
    chartCar = document.getElementById("chartCar");
    chartSection = document.getElementById("chartSection");
    chartDate = document.getElementById("chartDate");
    pass = document.getElementById('password');

    loadFrame = document.getElementById('load_frame');
    loadText = document.getElementById('load_text');
    connectFrame = document.getElementById('connect_frame');
    connectText = document.getElementById('connect_text');

    accessLabel = document.getElementById('accessLabel');

    paramArray = [par0, par1, par2, par3, par4, par5, par6, par7];
    loadWindow = [loadFrame, loadText];
    connectWindow = [connectFrame, connectText];
}

window.onload = function() {
    initParams();
    drawArrow();
    let jsObj = JSON.stringify({needAllData: true});
    connectToESP(jsObj, clearLoadWindow);
    setInterval(refresh, 750);
}

function multiActionClass(action, elementArray, classArray){
    for(let idx in elementArray){
        let element = elementArray[idx];
        for(let idx in classArray) {
            switch(action){
                case 'toggle':
                    element.classList.toggle(classArray[idx]);
                break;
                case 'add':
                    element.classList.add(classArray[idx]);
                break;
                case 'remove':
                    element.classList.remove(classArray[idx]);
                break;
            }
        }
    }
}

function clearLoadWindow(){
    setTimeout(multiActionClass, 2000, 'add', loadWindow, ['hidden']);
}

function setLoadWindow(){
    multiActionClass('remove', loadWindow, ['hidden']);
}

function setConnectWindow(){
    setTimeout(multiActionClass, 2000, 'remove', connectWindow, ['hidden']);
}

function clearConnectWindow(){
    multiActionClass('add', connectWindow, ['hidden']);
}

function refresh(){
    if (request == 0) connectToESP();
}

function setEmptyOption(selector, msgId){
    let msg;
    switch (msgId){
        case 0 : msg = 'Виберіть машину'; break;
        case 1 : msg = 'Виберіть секцію'; break;
        case 2 : msg = 'Виберіть дату'; break;
        default : msg = 'Виберіть машину'; break;
    }
    selector.innerHTML = '';
    let option = document.createElement('option');
    option.value = 'empty';
    option.innerHTML = msg;
    option.disabled = true;
    option.hidden = true;
    selector.appendChild(option);
    selector.disabled = true;
    selector.value = "empty";
}

function drawArrow(){
    let arrowImg = [
        {p : [{x:0 , y:32}, {x:190 , y:32}, {x:190 , y:118}, {x:0 , y:118}], c:'#ff0000'},
        {p : [{x:108 , y:0}, {x:108 , y:32}, {x:190 , y:32}], c:'#ff0000'},
        {p : [{x:108 , y:150}, {x:108 , y:118}, {x:190 , y:118}], c:'#ff0000'},
        {p : [{x:190 , y:32}, {x:190 , y:118}, {x:300 , y:75}], c:'#ff0000'},
    ];
    for(let i = 0; i <= 3; i++) drawLineElement('img_' + i,arrowImg);
}

function drawLineElement(id, object){
    for(let obj in object){
        let element = object[obj];
        let canvas = document.getElementById(id);
        let ctx = canvas.getContext('2d');
        if (element['c']) ctx.fillStyle = element['c'];
        ctx.beginPath();
        ctx.moveTo(element['p'][0].x,element['p'][0].y);
        for(let i in element['p']){
            if (i == 0) continue;
            let point = element['p'][i];
            ctx.lineTo(point.x,point.y);
        }
        ctx.fill();
    }
}

function refreshImage(){
    for (let i = 0; i<=3; i++){
        let img = document.getElementById('img_'+i);
        img.style['display'] = (current.dataset['plc'] == i) ? "block" : "none";
    }
}

function mainBAction(actionName, action){
    if (step == 0 && "startCButton" && !validInput(carN, 'car')) return;
    if (step == 6 && "startSButton" && !validInput(carS, 'section','number')) return;
    carN.disabled = true;
    carS.disabled = true;
    selector.disabled = true;
    if (step == 11 && actionName == 'doseButton') action = 'off';
    let button = {};
    button[actionName] = action;
    let jsObj = JSON.stringify(button);
    connectToESP(jsObj);
}

function setOption(idx){
    let option = (idx) ? selector.options[idx] : selector.options[0];
    option.selected = true;
}

function changeSettingWindow() {
    multiActionClass('toggle', [mainW,settingW], ['hidden']);
}

function changeRecordWindow() {
    multiActionClass('toggle', [settingW,recordW], ['hidden']);
}

function saveParam(){
    setLoadWindow();
    let obj = {};
    for (let idx in paramArray){
        let val = paramArray[idx].value;
        if (idx == 1) {val = (parseFloat(val)*1000).toString()};
        obj['param_'+idx] = val;
    }
    let jsObj = JSON.stringify(obj);
    connectToESP(jsObj, clearLoadWindow);
}

function saveCarNumber(){
    if (validCar) {
        setLoadWindow();
        let jsObj = JSON.stringify({car_number: carN.value});
        connectToESP(jsObj, clearLoadWindow);
    }
}
function saveCarSection(){
    if (validSection) {
        setLoadWindow();
        let jsObj = JSON.stringify({car_section: carS.value});
        connectToESP(jsObj, clearLoadWindow);
    }
}
function saveOption(){
    setLoadWindow();
    let jsObj = JSON.stringify({option: selector.options[selector.selectedIndex].value});
    connectToESP(jsObj, clearLoadWindow);
}

function checkPass() {
    let access = false;
    if (pass.value == "123") access = true;
    for (let idx in paramArray){
        paramArray[idx].disabled = !access;
    }
}

function setPLCValue(respObj){
    if (first){
        first = false;
        carN.value = respObj['car_number'];
        carS.value = respObj['car_section'];
        checkValidInputs();
        par0.value = respObj['param_0'];
        par1.value = (parseFloat(respObj['param_1'])/1000).toString();
        par2.value = respObj['param_2'];
        par3.value = respObj['param_3'];
        par4.value = respObj['param_4'];
        par5.value = respObj['param_5'];
        par6.value = respObj['param_6'];
        par7.value = respObj['param_7'];      
        for (let idx in paramArray){
            paramArray[idx].value = paramArray[idx].valueAsNumber;
        }
    }
    let label;
    switch(respObj['access']){
        case "guest": label = "Гість"; break;
        case "operator": label = "Оператор"; break;
    }
    accessLabel.innerHTML = label;
    setOption(respObj['option']);
    if (respObj['plc_connect'] == false) {setConnectWindow();} else {clearConnectWindow();}
    val0.innerHTML = parseFloat(respObj['val_0'])/1000;
    val1.innerHTML = parseFloat(respObj['val_1'])/1000;
    val2.innerHTML = parseFloat(respObj['val_2'])/1000;
    val3.innerHTML = parseFloat(respObj['val_3'])/1000;
    current.dataset['plc'] = respObj['current'];
    step = respObj['progStep'];
    currentSt.innerHTML = changeStatus(step);
    refreshImage();
}

function validInput(input, type, pattern) {
    let isValid = false;
    if (input.value != '') isValid = validText(input.value, pattern);
    switch(type){
        case 'car' : validCar = isValid; break;
        case 'section' : validSection = isValid; break;
        default : break;
    }
    input.style['background'] = isValid ? 'white' : 'red';
    return isValid;
}

let letter_sample = ['a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z'];
let number_sample = ['1','2','3','4','5','6','7','8','9','0'];
function validText(text, pattern){
    let isValid = true;
    for(let idx in text){
        charValid = false;
        let char = text[idx];
        for(let i in number_sample){
            let number = number_sample[i];
            if (char == number) {charValid = true; break;} 
        }
        if (pattern != "number") {
            if (charValid) continue;
            for(let i in letter_sample){
                let letter = letter_sample[i];
                if (char.toLowerCase() == letter) {charValid = true; break;} 
            }
        }
        if (!charValid) {isValid = false; break;}
    }
    return isValid;
}

function checkValidInputs(){
    validInput(carN, 'car');
    validInput(carS, 'section','number');
}

function createOption(parent, value){
    let option = document.createElement('option');
    option.value = option.innerHTML = value;
    parent.appendChild(option);
}

function connectToESP(jsObj, func) {
    setTimeout(sendPLC,request*50, jsObj, func);
}

function zeroConfirm(){
    if (step == 10){
        if (confirm('Ви впевнені, що хочете обнулити запис?')) {
            setLoadWindow();
            let button = {};
            button['zeroButton'] = 'on';
            let jsObj = JSON.stringify(button);
            connectToESP(jsObj, clearLoadWindow);
        } else {}
    }
}

function sendPLC(jsObj, func){
    request++;
    let uri = "/ESP";
    let xhr = new XMLHttpRequest();
    if (jsObj) {
        let date = new Date();
        let dateStr;
        dateStr = ("0" + date.getDate()).slice(-2) + "-";
        dateStr += ("0" + (date.getMonth()+1)).slice(-2) + "-";
        dateStr += date.getFullYear() + "_";
        dateStr += ("0" + (date.getHours()+date.getTimezoneOffset()/60)).slice(-2) + "-";
        dateStr += ("0" + date.getMinutes()).slice(-2) + "-";
        dateStr += ("0" + date.getSeconds()).slice(-2);
        uri += "?date="+dateStr;
        uri += "&json="+jsObj;
    } 
    xhr.open("POST", uri, true);
    xhr.func = func;
    xhr.onload = function () {
        if (this.status == 200){
            request--;
            if (this.response){
                let respObj = JSON.parse(this.response);
                setPLCValue(respObj);
                if (this.func) {this.func.call(this, respObj)};
            }
        }
    };
    xhr.send();
}

function logout(){
    window.location.assign("/LogIn");
}

function changeStatus(step){
    selector.disabled = false;
    carN.disabled = false;
    carS.disabled = false;
    multiActionClass('remove', [sb1,sb2,sb3,sb4,doseB], ['active']);
    switch(step){
        case 0:
            selector.disabled = true;
            carS.disabled = true;
            multiActionClass('add', [sb2,sb4], ['active']);
        return 'Стоп';
        case 4:
            selector.disabled = true;
            carN.disabled = true;
            carS.disabled = true;
            multiActionClass('add', [sb1,sb4], ['active']);
        return 'Підготовка';
        case 5:
            selector.disabled = true;
            carN.disabled = true;
            carS.disabled = true;
            multiActionClass('add', [sb1,sb4], ['active']);
        return 'Підготовка';
        case 6:
            selector.disabled = true;
            carN.disabled = true;
            multiActionClass('add', [sb1,sb4], ['active']);
        return 'Вибір секції';
        case 10:
            carN.disabled = true;
            carS.disabled = true;
            multiActionClass('add', [sb1,sb3], ['active']);
        return 'Очікування';
        case 11:
            selector.disabled = true;
            carN.disabled = true;
            carS.disabled = true;
            multiActionClass('add', [sb1,sb3,doseB], ['active']);
        return 'Подача води';
        case 12:
            selector.disabled = true;
            carN.disabled = true;
            carS.disabled = true;
            multiActionClass('add', [sb1,sb3], ['active']);
        return 'Зупинка';
        case 20:
            selector.disabled = true;
            carN.disabled = true;
            carS.disabled = true;
            multiActionClass('add', [sb2,sb4], ['active']);
        return 'Завершення';
        case 21:
            selector.disabled = true;
            carN.disabled = true;
            carS.disabled = true;
            multiActionClass('add', [sb2,sb4], ['active']);
        return 'Завершення';
    }
}

function getChartRecords(){
    let uri = "/getRecords";
    let xhr = new XMLHttpRequest();
    xhr.open("POST", uri, true);
    xhr.onload = function () {
        if (this.status == 200){
            parseChartData(this.response);
        }
    };
    xhr.send();
}

function getRecordData(){
    let uri = "/getRecordData";
    let xhr = new XMLHttpRequest();
    uri += "?src="+chartDate.value+"_"+chartCar.value+"_"+chartSection.value+".txt";
    xhr.open("POST", uri, true);
    xhr.onload = function () {
        if (this.status == 200){
            buildChart(this.response);
        }
    };
    xhr.send();
}

function parseChartData(resp){
    let records = resp.split('/');
    for(let i in records){
    // 0 - date, 1 - time, 2 - car, 3 - section
    let record = records[i];
    if (record == "") continue;
    let data = record.split('_');
    data[3] = data[3].split('.')[0];
    if (data[0] == undefined || data[1] == undefined || data[2] == undefined || data[3] == undefined) {chartData = {}; return;}
    let date = data[0] + "_" + data[1];
    if (!chartData[data[2]]) chartData[data[2]] = {};
    if (!chartData[data[2]][data[3]]) chartData[data[2]][data[3]] = {};
    chartData[data[2]][data[3]][date] = true;
    }
    for(let car in chartData) createOption(chartCar,car);
    if (chartRefresh) reloadChartData();
    clearLoadWindow();
}
    
function setCar(carId){
    document.getElementById('chartDiv').innerHTML = "";
    setEmptyOption(chartSection, 1);
    chartSection.disabled = false;
    setEmptyOption(chartDate, 2)
    currentChart = {'car' : carId, 'section' : '', 'date' : ''}; 
    for(let section in chartData[carId]) createOption(chartSection,section);
}
    
function setSection(sectionId){
    document.getElementById('chartDiv').innerHTML = "";
    setEmptyOption(chartDate, 2);
    chartDate.disabled = false;
    currentChart = {'car' : currentChart['car'], 'section' : sectionId, 'date' : ''}; 
    for(let date in chartData[currentChart['car']][sectionId]) createOption(chartDate,date);
}
    
function setDate(DateId){
    document.getElementById('chartDiv').innerHTML = "";
    currentChart = {'car' : currentChart['car'], 'section' : currentChart['section'], 'date' : DateId};
}

function buildChart(resp){
    let valArray = resp.split(' ');
    let log = document.getElementById('chartDiv');
    log.innerHTML = "";
    for(let idx in valArray){
    if (valArray[idx] == "") break;
    let val;
    if (idx != 0) {
    log.innerHTML += "<br>";
    val = parseFloat(valArray[idx]) - parseFloat(valArray[idx-1]);
    } else {
    val = parseFloat(valArray[idx]);
    }
    log.innerHTML += "Налив " + (parseInt(idx) + 1) + " - " + val/1000 + " літрів"; 
    }
    clearLoadWindow();
}
    
function refreshChart(){
    setEmptyOption(chartCar, 0);
    chartCar.disabled = false;
    setEmptyOption(chartSection, 1);
    setEmptyOption(chartDate, 2);
    chartData = {};
    chartRefresh = true;
    getChartRecords();
}
    
function reloadChartData(){
    chartRefresh = false;
    let mem = currentChart;
    if (mem['car'] == "" || chartData[mem['car']] == undefined) return;
    setCar(mem['car']);
    chartCar.value = mem['car'];
    if (mem['section'] == "" || chartData[mem['car']][mem['section']] == undefined) return;
    setSection(mem['section']);
    chartSection.value = mem['section'];
    if (mem['date'] == "" || chartData[mem['car']][mem['section']][mem['date']] == undefined) return;
    setDate(mem['date']);
    chartDate.value = mem['date'];
    getRecordData();
}

