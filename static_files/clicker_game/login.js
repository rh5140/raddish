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

export const createAccount = async (_username, _password) => {
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


export const loginAccount = async (_username, _password) => {
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


export const logoutAccount = async (_username, _session_id, _radish_num, _upgrades) => {
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



