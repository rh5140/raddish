//FETCH VERSIONS

//https://web.dev/articles/introduction-to-fetch

function checkStatus(response) {
  if (response.status >= 200 && response.status < 300) {
  return Promise.resolve(response)
  } else {
  return Promise.reject(new Error(response.statusText)) //todo: maybe better error reporting so we know what happened
  }
}

function json(response) {
  return response.json()
}

const createAccount = async (_username, _password) => {
  return new Promise(function (fulfill, reject){
    let bodyString =  JSON.stringify({
      username : _username,
      password : _password,
      action : "create"
    });
    console.log("Body String (create): ", bodyString);
    fetch('./data',{
      method: 'post',
      headers: {
        "Content-Type":"application/json; charset=UTF-8"
      },
      body: bodyString
    })
    .then(checkStatus)
    .then(function(data){
      console.log("Account creation successful.");
      fulfill(data);
    })
    .catch(function (error) {
      console.log('Request failed', error);
      reject(error);
    });
  })
}


const loginAccount = async (_username, _password) => {
  let bodyString =  JSON.stringify({
    username : _username,
    password : _password,
    action : "login"
  });
  console.log("Body String (login): ", bodyString);
  return new Promise(function (fulfill, reject){
    fetch('./data',{
      method: 'post',
      headers: {
        "Content-Type":"application/json; charset=UTF-8"
      },
      body: bodyString
    })
    .then(checkStatus)
    .then(json)
    .then(function(data){
      console.log("Good Request: ", data);
      fulfill(data);
    })
    .catch(function (error) {
      console.log('Request failed', error);
      reject(error);
    });
  })
}


const logoutAccount = async (_username, _session_id, _radish_num, _upgrades) => {
  console.log(_username);
  let bodyStr = JSON.stringify({
    username: _username,
    session_id: _session_id,
    radish_num: _radish_num,
    upgrades: _upgrades
  });

  console.log("Body String (logout): ", bodyStr);
  return new Promise(function (fulfill, reject){
    fetch('./data',{
      method: 'put',
      headers: {
        "Content-Type":"application/json; charset=UTF-8"
      },
      body: bodyStr
    })
    .then(checkStatus)
    .then(function(data){
      console.log("Logout successful.");
      fulfill(data);
    })
    .catch(function (error) {
      console.log('Request failed', error);
      reject(error);
    });
  })
}



//Login/Register form code
//form open/close
document.getElementById('loginButton').addEventListener('click', openForm);
document.getElementById('closeButton').addEventListener('click', closeForm);

function openForm() {
  document.getElementById("loginFormBox").style.display = "block";
}

function closeForm() {
  document.getElementById("loginFormBox").style.display = "none";
} 


//form

const loginForm = document.getElementById("loginForm");
loginForm.addEventListener("submit", handleForm);

let _session_id; //need to store this to log out
let _session_username;

async function handleForm(event) {
  console.log(event.submitter.value);
  event.preventDefault();
  console.log(loginForm);
  const { username, password } = loginForm.elements;
  const userText = username.value.trim(); //removes whitespace at end
  const userPass = password.value.trim();
  if(event.submitter.value == "create"){
    try{
      await createAccount(userText, userPass);
      let data = await loginAccount(userText, userPass);
      console.log(data);
      _session_username = userText;
      _session_id = data.session_id;
      alert("Creation successful!");
      setErrorText("");
      closeForm();
      loginForm.reset();
    }
    catch(err){ //promise reject
      setErrorText("Username already exists");
      console.log(err);
    }

  }
  else{
    try{
      let data = await loginAccount(userText, userPass);
      console.log(data);
      _session_username = userText;
      _session_id = data.session_id;
      alert("Login successful!");
      setErrorText("");
      closeForm();
      loginForm.reset();
    }
    catch(err){ //promise reject
      setErrorText("Incorrect username/password");
      console.log(err);
    }

  }
}


const errorMessages = document.getElementById("loginFormError");
function setErrorText(text){
  errorMessages.innerHTML = text;
}



//logout button
document.getElementById('logoutButton').addEventListener('click', doLogout);

async function doLogout(){
  try{
    await logoutAccount(_session_username, _session_id, 0, {}) //todo: get the other numbers]
    alert("Logout Successful!");
  }
  catch{

  }
}