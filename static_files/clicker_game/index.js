import { createAccount, loginAccount, logoutAccount } from "./login.js";

let cardMap = new Map();

// vars 
let radishCount = 0; //temp
let moneyCount = 0;
let farmerCount = 0;
let farmCount = 0;
let taffyCount = 0;
let lobsterCount = 0;
let almondCount = 0;
let hasCharlie = false;
let hasPowell = false;

// constants
const INITIAL_COUNT_ZERO    =    0;
const FARMER_GAIN           =    1;
const FARM_GAIN             =   10;
const INITIAL_FARMER_COST   =    100;
const INITIAL_FARM_COST     =   1000;
const INITIAL_TAFFY_COST    =   2000;
const CHARLIE_COST          =   4000;
const INITIAL_ALMOND_COST   =  20000;
const INITIAL_LOBSTER_COST  =  50000;
const POWELL_COST           = 100000;
const FARMER_THRESHOLD      =  100;
const EXPONENT_BASE = 1.15;

//price fluctuation
let priceFluctuationTimer = 0;
let currentRadishPrice = 1.00.toFixed(2);
let baseRadishPrice = 1;

// Actual max/min multiplied by 100 to make calculation easier
let priceMax = 1.25 * 100;
let priceMin = 0.75 * 100;
const LOBSTER_GAIN = 0.20 * 100;

// Nodes
const moneyCountNode = document.getElementById("moneyCount");
const radishCountNode = document.getElementById("radishCount");
const radishPriceNode = document.getElementById("radishPrice");
const radishPerSecondNode = document.getElementById("radishPerSecond");
const priceRangeNode = document.getElementById("priceRange");

// list of radish img sources for animation (currently 11 upgrades from the base radish)
const radishes = ["images/radish.png", 
                  "images/radishes/daikon.png", 
                  "images/radishes/longscarlet.png", 
                  "images/radishes/summersicle.png", 
                  "images/radishes/redking.png", 
                  "images/radishes/sparkler.png", 
                  "images/radishes/misatorose.png", 
                  "images/radishes/malaga.png", 
                  "images/radishes/luobo.png", 
                  "images/radishes/ladyslipper.png", 
                  "images/radishes/helios.png", 
                  "images/radishes/rattail.png"];

const main = () => {
    console.log("index loaded!");
    setInterval(updateLoop, 1000); //1 second "tick" for now

    document.querySelector("#radishImg").addEventListener("click", onRadishClick);

    document.querySelector("#sellRadish").addEventListener("click", sellRadish.bind(null, 1));
    document.querySelector("#sellRadishTen").addEventListener("click", sellRadish.bind(null, 10));
    document.querySelector("#sellRadishHundred").addEventListener("click", sellRadish.bind(null, 100));
    generateMarket();
    // TODO: Update values in cardMap (id -> struct) based on information stored about a user
    updateAllUI();
};


const onRadishClick = (event) => {
    // powellModifier = 1 if no Powell, so just the same as a click
    let powellModifier = 1 + (hasPowell * Math.floor(farmerCount / FARMER_THRESHOLD));
    radishCount += (powellModifier); // If Powell is bought, click adds 1 more radish for every 100 farmers bought
    updateRadishCountUI(); 
    radishAnimation();
}

function radishAnimation() {
    // Animations referenced from https://stackoverflow.com/questions/69970533/create-a-1-animation-when-button-is-clicked
    let clickAnimation = document.createElement("img");
    let radishIdx;
    if (farmCount + 1 > radishes.length) {
        radishIdx = Math.floor(Math.random() * (radishes.length));
    }
    else {
        radishIdx = Math.floor(Math.random() * (farmCount + 1));
    }
    clickAnimation.src = radishes[radishIdx];

    clickAnimation.width = 50;
    clickAnimation.height = 50;
    document.getElementById("clickBox").appendChild(clickAnimation);
    clickAnimation.classList.add("clickAnimation"); // Add the class that animates

}

//main event loop
const updateLoop = () => {
    radishCount += calcRadishPerSecond();

    priceFluctuationTimer += 1; //happens every 10 seconds for now
    if(priceFluctuationTimer >= 10){
        priceFluctuationTimer = 0;
        changeRadishPrice();
    }
    updateRadishPerSecondUI();
    updateRadishPriceUI();
    updateRadishCountUI();
};

function updateAllUI() {
    updateMoneyCountUI();
    updateRadishPerSecondUI();
    updateRadishPriceUI();
    updateRadishCountUI();
    updateMarketUI();
    updatePriceRangeUI();
}

function updateMoneyCountUI() {
    moneyCountNode.innerText = "$" + moneyCount.toFixed(2);
}
function updateRadishCountUI() {
    radishCountNode.innerText = radishCount;
}
function updateRadishPerSecondUI() {
    radishPerSecondNode.innerText = calcRadishPerSecond();
}
function updateRadishPriceUI() {
    radishPriceNode.innerText = "$" + currentRadishPrice;
}
function updatePriceRangeUI() {
    priceRangeNode.innerText = "$" + (priceMin / 100).toFixed(2) + "-$" + ((priceMax + lobsterCount * LOBSTER_GAIN) / 100).toFixed(2);
}

const calcRadishPerSecond = () =>{
    // Each taffy increases each individual farmer's RPS by 1
    let taffyModifier = taffyCount + 1;
    let farmerRPS = farmerCount * FARMER_GAIN * taffyModifier;
    // If Charlie is bought, each almond increases each individual farm's RPS by 10
    let farmModifier = 1 + (hasCharlie * almondCount);
    let farmRPS = farmCount * FARM_GAIN * farmModifier;
    // If Powell is bought, flat multiplier that increases by 1 for every FARMER_THRESHOLD farmers owned
    let powellModifier = 1 + (hasPowell * Math.floor(farmerCount / FARMER_THRESHOLD));

    return powellModifier * (farmerRPS + farmRPS);
}


const changeRadishPrice = () => {
    let priceVariance = Math.floor(Math.random() * (priceMax - priceMin + 1)) + priceMin;
    priceVariance = priceVariance / 100;
    currentRadishPrice = (baseRadishPrice * priceVariance).toFixed(2);
}

const sellRadish = (amount) =>{
    console.log(amount);
    if(radishCount >= amount){
        radishCount -= amount;
        moneyCount += amount * currentRadishPrice;
    }
    else {
        // TODO: Visual indicator for not being able to sell
    }
    updateRadishCountUI();
    updateMoneyCountUI();
}

function marketCard(name, image, alt, description, initialCost, cost, count) {
    this.name = name;
    this.image = image;
    this.alt = alt;
    this.description = description;
    this.initialCost = initialCost;
    this.cost = cost;
    this.count = count;
}

function generateCardArray() {
    let cardArray = [];
    cardArray.push(new marketCard("Farmer", "images/cat_farmer.png", "A drawing of a wide-eyed black cat wearing a tan farmer hat and overalls", 
                                  "Works hard to help you farm MEOWntains of radishes. Each farmer contributes a base 1 RPS.", 
                                  INITIAL_FARMER_COST, INITIAL_FARMER_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Farm", "images/farm.png", "A drawing of a three-row radish plot", 
                                  "More farms means more radishes. Each farm contributes a base 100 RPS.", 
                                  INITIAL_FARM_COST, INITIAL_FARM_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Taffy", "images/taffy.png", "A drawing of an open candy wrapper with the text \'CAT-SAFE TAFFY\'. There is a tan piece of taffy sticking out.", 
                                "Motivates your farmers to work harder. Each taffy fed increases the farmer RPS multiplier by 1.", 
                                INITIAL_TAFFY_COST, INITIAL_TAFFY_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Charlie", "images/charlie.png", "A drawing of a gold-capped conure, a parakeet with a red head, black beak, and green body.", 
                                  "Charlie's poop makes for great fertilizer.", 
                                  CHARLIE_COST, CHARLIE_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Almond", "images/almond.png", "A drawing of an almond in its shell.", 
                                  "Charlie loves cracking open almonds! Each almond Charlie eats increments the farm RPS multiplier by 1.", 
                                  INITIAL_ALMOND_COST, INITIAL_ALMOND_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Lobster", "images/lobster.png", "A drawing of a muted red lobster.", 
                                  "Mysteriously improves your radish quality. Each lobster increases the price maximum by $0.20.", 
                                  INITIAL_LOBSTER_COST, INITIAL_LOBSTER_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Powell", "images/powell.png", "A drawing of the head of a black and white tuxedo cat, surrounded by blue flowers.", 
                                  "Grows more powerful with more cats. Every 100 farmer increases both the total RPS and click multipliers by 1.", 
                                  POWELL_COST, POWELL_COST, INITIAL_COUNT_ZERO));

    return cardArray;
}

function buyUpgrade(cardId) {

    // Updating variables using struct
    const cardStruct = cardMap.get(cardId);
    let cost = cardStruct.cost;
    if (cost > moneyCount) {
        // TODO: Add visual feedback for not having enough money to buy an upgrade
        return;
    }
    moneyCount -= cost;
    updateMoneyCountUI();

    cardStruct.count += 1;
    // Check if unique
    switch (cardId) {
        case "Charlie":
            hasCharlie = true;
            buyUniqueUpgrade("Charlie", hasCharlie);
            return;
        case "Powell":
            hasPowell = true;
            buyUniqueUpgrade("Powell", hasPowell);
            return;
        default:
            break;
    }

    updateUpgradeUI(cardId);

    switch(cardId) {
        case "Farmer":
            farmerCount += 1;
            break;
        case "Farm":
            farmCount += 1;
            break;
        case "Taffy":
            taffyCount += 1;
            break;
        case "Lobster":
            lobsterCount += 1;
            priceMax += LOBSTER_GAIN;
            updatePriceRangeUI();
            break;
        case "Almond":
            almondCount += 1;
            break;
        default:
            break;
    }
    updateRadishPerSecondUI();
}

function calculateUpgradeCost(initialCost, count) {
    return Math.floor(initialCost * EXPONENT_BASE ** (count));
}

function updateUpgradeUI(name) {
    const cardStruct = cardMap.get(name);
    const initialCost = cardStruct.initialCost;
    const count = cardStruct.count;
    const cardNode = document.getElementById(name);
    let updatedCost = calculateUpgradeCost(initialCost, count);
    cardNode.querySelector(".upgrade_price").innerText = "Cost: $" + updatedCost;
    cardNode.querySelector(".upgrade_count").innerText = "Number: " + count;
    cardStruct.cost = updatedCost;
}

// Could probably be one function w/ updateUpgradeUI if I implemented better
// The one difference is that I need to get the global count variable here since that's what the db interacts with
function resetUpgradeUI(name, count) {
    const cardStruct = cardMap.get(name);
    const initialCost = cardStruct.initialCost;
    const cardNode = document.getElementById(name);
    let updatedCost = calculateUpgradeCost(initialCost, count);
    cardNode.querySelector(".upgrade_price").innerText = "Cost: $" + updatedCost;
    cardNode.querySelector(".upgrade_count").innerText = "Number: " + count;
    cardStruct.count = count;
    cardStruct.cost = updatedCost;

}

function buyUniqueUpgrade(name, bought) {
    updateUniqueUpgradeUI(name, bought);
    updateRadishPerSecondUI();
}

function updateUniqueUpgradeUI(name, bought) {
    let upgrade = document.getElementById(name);
    if (bought) {
        upgrade.querySelector(".buy_button").className = "bought_button";
        upgrade.querySelector(".bought_button").innerText = "Bought";
        upgrade.querySelector(".upgrade_price").innerText = "";
        upgrade.querySelector(".upgrade_count").innerText = "";
    }
    else {
        upgrade.querySelector(".bought_button").className = "buy_button";
        upgrade.querySelector(".buy_button").innerText = "Buy";
        let cost;
        switch(name) {
            case "Charlie":
                cost = CHARLIE_COST;
                break;
            case "Powell":
                cost = POWELL_COST;
                break;
            default:
                break;
        }
        upgrade.querySelector(".upgrade_price").innerText = "Cost: $" + cost;
        upgrade.querySelector(".upgrade_count").innerText = "Number: 0";
    }
}

// Very scuffed because of my having both the global variables and cardMap
function updateMarketUI() {
    for (const [name, struct] of cardMap) {
        switch(name) {
            case "Charlie":
                updateUniqueUpgradeUI("Charlie", hasCharlie);
                break;
            case "Powell":
                updateUniqueUpgradeUI("Powell", hasPowell);
                break;
            case "Farmer":
                resetUpgradeUI("Farmer", farmerCount);
                break;
            case "Farm":
                resetUpgradeUI("Farm", farmCount);
                break;
            case "Taffy":
                resetUpgradeUI("Taffy", taffyCount);
                break;
            case "Lobster":
                resetUpgradeUI("Lobster", lobsterCount);
                break;
            case "Almond":
                resetUpgradeUI("Almond", almondCount);
                break
            default:
                // Would love to make a default case that covers all of the other upgrades, but because of my bad implementation we can't
                break;
        }
    }
}

function generateMarket() {
    const base = document.getElementById("Base");
    const marketNode = document.getElementById("market");

    let cardArray = generateCardArray();
    for (let i = 0; i < cardArray.length; i++) {
        let clone = base.cloneNode(true);
        let card = cardArray[i];
        let name = card.name;
        clone.setAttribute("id", name);
        clone.querySelector(".upgrade_name").innerText = name;
        clone.querySelector(".upgrade_img").src = card.image;
        clone.querySelector(".upgrade_img").alt = card.alt;
        clone.querySelector(".upgrade_info").innerText = card.description;
        clone.querySelector(".upgrade_price").innerText = "Cost: $" + card.cost;
        clone.querySelector(".upgrade_price").setAttribute("id", name + "Cost");
        clone.querySelector(".upgrade_count").innerText = "Number: " + card.count;
        clone.querySelector(".upgrade_count").setAttribute("id", name + "Count");
        marketNode.appendChild(clone);

        cardMap.set(cardArray[i].name, cardArray[i]);
    }

    let cards = document.getElementsByClassName("card");
    for (let card of cards) {
        card.querySelector(".buy_button").addEventListener("click", function() {buyUpgrade(card.id)});
    }
}

//login/logout/etc stuff (done here because we set globals here)
//Login/Register form code
//form open/close
const closeButton = document.getElementById('closeButton')
const loginButton = document.getElementById('loginButton')
const logoutButton = document.getElementById('logoutButton');

loginButton.addEventListener('click', openForm);
closeButton.addEventListener('click', closeForm);



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
      //don't load radishes and other data since we JUST made the account
      setErrorText("");
      //swap which button is displayed
      logoutButton.style.display = "inline-block";
      loginButton.style.display = "none";
      //close and reset form
      closeForm();
      loginForm.reset();
      alert("Creation successful!");
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
      //load data
      if(data.radish_num){
        radishCount = data.radish_num; //load radishes
      }

      if(data.upgrades.moneyCount){
        moneyCount = data.upgrades.moneyCount;
      }
      if(data.upgrades.farmerCount){
        farmerCount = data.upgrades.farmerCount;
      }
      if(data.upgrades.farmCount){
        farmCount = data.upgrades.farmCount;
      }
      if(data.upgrades.taffyCount){
        taffyCount = data.upgrades.taffyCount;
      }
      if(data.upgrades.lobsterCount){
        lobsterCount = data.upgrades.lobsterCount;
      }
      if(data.upgrades.almondCount){
        almondCount = data.upgrades.almondCount;
      }
      if(data.upgrades.hasCharlie){
        hasCharlie = data.upgrades.hasCharlie;
      }
      if(data.upgrades.hasPowell){
        hasPowell = data.upgrades.hasPowell;
      }
      //do upgrades as needed
      //if we keep adding upgrades we maybe need to find a better method
      //swap which button is displayed
      logoutButton.style.display = "inline-block";
      loginButton.style.display = "none";
      setErrorText("");
      closeForm();
      loginForm.reset();
      updateAllUI();
      alert("Login successful!");
    }
    catch(err){ //promise reject
      setErrorText("Incorrect username/password");
      console.log(err);
    }

  }
}

//login/create error
const errorMessages = document.getElementById("loginFormError");
function setErrorText(text){
  errorMessages.innerHTML = text;
}

//logout button
logoutButton.addEventListener('click', doLogoutAlert);

async function doLogoutAlert(){
  try{
    if(await doLogout()){
      alert("Logout Successful!");
    }
  }
  catch(err){

  }
}

async function doLogout(){
  try{
    let upgrades_json = updateUpgradesJson();
    await logoutAccount(_session_username, _session_id, radishCount, upgrades_json) 
    //swap which button is displayed
    logoutButton.style.display = "none";
    loginButton.style.display = "inline-block";
    resetVars();
    updateAllUI();
    alert("Logout Successful!");
  }
  catch{

  }
}

function updateUpgradesJson() {
    let upgrades_json = {}
    upgrades_json["moneyCount"] = moneyCount;
    upgrades_json["farmerCount"] = farmerCount;
    upgrades_json["farmCount"] = farmCount;
    upgrades_json["taffyCount"] = taffyCount;
    upgrades_json["lobsterCount"] = lobsterCount;
    upgrades_json["almondCount"] = almondCount;
    upgrades_json["hasCharlie"] = hasCharlie;
    upgrades_json["hasPowell"] = hasPowell;
    return upgrades_json;
}

function resetVars() {
    console.log("reset vars");
    radishCount = 0;
    moneyCount = 0;
    farmerCount = 0;
    farmCount = 0;
    taffyCount = 0;
    lobsterCount = 0;
    almondCount = 0;
    hasCharlie = false;
    hasPowell = false;
    priceMax = 1.25 * 100;
    return true;
  }
  catch{
    return false;
  }
}




//force logout on window close
window.addEventListener("beforeunload", doLogout);

main();