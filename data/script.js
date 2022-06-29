
let request = 0;
let step;
let maxNReq = 20;
let validCar = true;
let validSection = true;
let numParam = 8;
let first = true;

let mainW, settingW, pass, loadFrame, loadText;
let loadWindow = [];

function initParams(){
    mainW = document.getElementById("mainW");
    settingW = document.getElementById("settingW");
    chartW = document.getElementById("chartW");
    pass = document.getElementById('password');

    loadFrame = document.getElementById('load_frame');
    loadText = document.getElementById('load_text');
    loadWindow = [loadFrame, loadText];
}

window.onload = function() {
    initParams();
    drawArrow();
    let jsObj = JSON.stringify({needAllData: true});
    connectToESP(jsObj);
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
    let current = document.getElementById('current').dataset['plc'];
    for (let i = 0; i<=3; i++){
        let img = document.getElementById('img_'+i);
        img.style['display'] = (current == i) ? "block" : "none";
    }
}

function mainBAction(actionName, action){
    document.getElementById('car_number').disabled = true;
    document.getElementById('car_section').disabled = true;
    document.getElementById('option').disabled = true;
    if (step == 11 && actionName == 'doseButton') action = 'off';
    let button = {};
    button[actionName] = action;
    let jsObj = JSON.stringify(button);
    connectToESP(jsObj);
}

function setOption(idx){
    let select = document.getElementById('option');
    let option = (idx) ? select.options[idx] : select.options[0];
    option.selected = true;
}

function changeSettingWindow() {
    mainW.style['display'] = mainW.style['display'] == 'none' ? 'block' :'none';
    settingW.style['display'] = settingW.style['display'] == 'block' ? 'none' : 'block';
}

function saveParam(){
    setLoadWindow();
    let obj = {};
    for (let i = 0 ; i < numParam; i++) {
        let par = document.getElementById('param_'+i);
        let val = par.value;
        if (i == 1) {val = (parseFloat(val)*1000).toString()};
        obj['param_'+i] = val;
    }; 
    let jsObj = JSON.stringify(obj);
    connectToESP(jsObj, clearLoadWindow);
}

function saveCarNumber(){
    if (validCar) {
        setLoadWindow();
        let jsObj = JSON.stringify({car_number: document.getElementById('car_number').value});
        connectToESP(jsObj, clearLoadWindow);
    }
}
function saveCarSection(){
    if (validSection) {
        setLoadWindow();
        let jsObj = JSON.stringify({car_section: document.getElementById('car_section').value});
        connectToESP(jsObj, clearLoadWindow);
    }
}
function saveOption(){
    setLoadWindow();
    let selector = document.getElementById('option');
    let jsObj = JSON.stringify({option: selector.options[selector.selectedIndex].value});
    connectToESP(jsObj, clearLoadWindow);
}

function checkPass() {
    let access = false;
    if (pass.value == "123") access = true;
    for (let i = 0; i < numParam; i++){
        let param = document.getElementById('param_'+i);
        param.disabled = !access;
    }
}

function setPLCValue(respObj){
    if (first){
        first = false;
        document.getElementById('car_number').value = respObj['car_number'];
        document.getElementById('car_section').value = respObj['car_section'];
        document.getElementById('param_0').value = respObj['param_0'];
        document.getElementById('param_1').value = (parseFloat(respObj['param_1'])/1000).toString();
        document.getElementById('param_2').value = respObj['param_2'];
        document.getElementById('param_3').value = respObj['param_3'];
        document.getElementById('param_4').value = respObj['param_4'];
        document.getElementById('param_5').value = respObj['param_5'];
        document.getElementById('param_6').value = respObj['param_6'];
        document.getElementById('param_7').value = respObj['param_7'];      
        for (let i = 0; i < numParam; i++){
            let param = document.getElementById('param_'+i);
            param.value = param.valueAsNumber;
        }
        checkValidInputs();
        clearLoadWindow();
    }
    setOption(respObj['option']);
    document.getElementById('val_0').innerHTML = parseFloat(respObj['val_0'])/1000;
    document.getElementById('val_1').innerHTML = parseFloat(respObj['val_1'])/1000;
    document.getElementById('val_2').innerHTML = parseFloat(respObj['val_2'])/1000;
    document.getElementById('val_3').innerHTML = parseFloat(respObj['val_3'])/1000;
    document.getElementById('current').dataset['plc'] = respObj['current'];
    step = respObj['progStep'];
    document.getElementById('status').innerHTML = changeStatus(step);
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
    validInput(document.getElementById('car_number'), 'car');
    validInput(document.getElementById('car_section'), 'section','number');
}

function createOption(parent, value){
    let option = document.createElement('option');
    option.value = option.innerHTML = value;
    parent.appendChild(option);
}

function connectToESP(jsObj, func) {
    // if (!func) func = clearLoadWindow;
    setTimeout(sendPLC,request, jsObj, func);
}

function zeroConfirm(){
    if (step == 10){
        if (confirm('Ви впевнені, що хочете обнулити запис?')) {
            setLoadWindow();
            mainBAction('zeroButton', 'On');
        } else {}
    }
}

function sendPLC(jsObj, func){
    let xhr = new XMLHttpRequest();
    xhr.open("POST", "/Connect", true);
    xhr.func = func;
    xhr.onload = function () {
        if (this.response){
            let respObj = JSON.parse(this.response);
            setPLCValue(respObj);
            this.func.call(this, respObj);
        }
    };
    xhr.send(jsObj);
}

function changeStatus(step){
    document.getElementById('car_number').disabled = true;
    document.getElementById('car_section').disabled = true;
    document.getElementById('option').disabled = true;
    document.getElementById('sb1').classList[0] = "";
    document.getElementById('sb1').classList.value = "";
    document.getElementById('sb2').classList[0] = "";
    document.getElementById('sb2').classList.value = "";
    document.getElementById('sb3').classList[0] = "";
    document.getElementById('sb3').classList.value = "";
    document.getElementById('sb4').classList[0] = "";
    document.getElementById('sb4').classList.value = "";
    multiActionClass('remove', [document.getElementById('doseB')], ['active']);
    // document.getElementById('doseB').classList[0] = "";
    // document.getElementById('doseB').classList.value = "";
    switch(step){
        case 0: 
            document.getElementById('car_number').disabled = false;
            document.getElementById('sb2').classList[0] = "active";
            document.getElementById('sb2').classList.value = "active";
            document.getElementById('sb4').classList[0] = "active";
            document.getElementById('sb4').classList.value = "active";
        return 'Стоп';
        case 4: 
            document.getElementById('sb1').classList[0] = "active";
            document.getElementById('sb1').classList.value = "active";
            document.getElementById('sb4').classList[0] = "active";
            document.getElementById('sb4').classList.value = "active";
        return 'Підготовка';
        case 5: 
            document.getElementById('sb1').classList[0] = "active";
            document.getElementById('sb1').classList.value = "active";
            document.getElementById('sb4').classList[0] = "active";
            document.getElementById('sb4').classList.value = "active";
        return 'Підготовка';
        case 6: 
            document.getElementById('car_section').disabled = false;
            document.getElementById('sb1').classList[0] = "active";
            document.getElementById('sb1').classList.value = "active";
            document.getElementById('sb4').classList[0] = "active";
            document.getElementById('sb4').classList.value = "active";
        return 'Вибір секції';
        case 10:
            document.getElementById('option').disabled = false;
            document.getElementById('sb1').classList[0] = "active";
            document.getElementById('sb1').classList.value = "active";
            document.getElementById('sb3').classList[0] = "active";
            document.getElementById('sb3').classList.value = "active";
        return 'Очікування';
        case 11: 
            document.getElementById('sb1').classList[0] = "active";
            document.getElementById('sb1').classList.value = "active";
            document.getElementById('sb3').classList[0] = "active";
            document.getElementById('sb3').classList.value = "active";
            multiActionClass('add', [document.getElementById('doseB')], ['active']);
        return 'Подача води';
        case 12: 
            document.getElementById('sb1').classList[0] = "active";
            document.getElementById('sb1').classList.value = "active";
            document.getElementById('sb3').classList[0] = "active";
            document.getElementById('sb3').classList.value = "active";
        return 'Зупинка';
        case 20:
            document.getElementById('sb2').classList[0] = "active";
            document.getElementById('sb2').classList.value = "active";
            document.getElementById('sb4').classList[0] = "active";
            document.getElementById('sb4').classList.value = "active";
        return 'Завершення';
        case 21:
            document.getElementById('sb2').classList[0] = "active";
            document.getElementById('sb2').classList.value = "active";
            document.getElementById('sb4').classList[0] = "active";
            document.getElementById('sb4').classList.value = "active";
        return 'Завершення';
    }
}
