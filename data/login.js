function logIn(){
    let jsObj = JSON.stringify({test: true}); 
    let xhr = new XMLHttpRequest();
    xhr.open("POST", "/LogIn", true);
    xhr.onload = function () {
        location.reload();
    };
    xhr.send(jsObj);
}