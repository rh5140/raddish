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
const INITIAL_FARMER_COST   =   10;
const INITIAL_TAFFY_COST    =   20;
const INITIAL_FARM_COST     =  100;
const INITIAL_ALMOND_COST   =  200;
const INITIAL_LOBSTER_COST  =  500;
const CHARLIE_COST          =  200;
const POWELL_COST           = 1000;
const FARMER_THRESHOLD      =  100;
const EXPONENT_BASE = 1.15;

//price fluctuation
let priceFluctuationTimer = 0;
let currentRadishPrice = 1;
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


const main = () => {
    console.log("index loaded!");
    setInterval(updateLoop, 1000); //1 second "tick" for now

    // document.querySelector("#radishClick").addEventListener("click", onRadishClick);
    document.querySelector("#radishImg").addEventListener("click", onRadishClick);

    document.querySelector("#sellRadish").addEventListener("click", sellRadish.bind(null, 1));
    document.querySelector("#sellRadishTen").addEventListener("click", sellRadish.bind(null, 10));
    document.querySelector("#sellRadishHundred").addEventListener("click", sellRadish.bind(null, 100));
    
    const cardMap = generateMarket();
    // TODO: Update values in cardMap (id -> struct) based on information stored about a user
    // Only need to store number of upgrades since cost can be calculated from number
};


const onRadishClick = (event) => {
    let powellModifier = 1 + (hasPowell * Math.floor(farmerCount / FARMER_THRESHOLD));
    radishCount += (1 + powellModifier); // If Powell is bought, click adds 1 more radish for every 100 farmers bought
    radishCountNode.innerText = "Radishes: " + radishCount.toString();
    // TODO: Animations?
}

//main event loop
const updateLoop = () => {
    radishCount += calcRadishPerSecond();

    priceFluctuationTimer += 1; //happens every 10 seconds for now
    if(priceFluctuationTimer >= 10){
        priceFluctuationTimer = 0;
        changeRadishPrice();
    }
    radishPerSecondNode.innerText = "Radishes/Sec: " + calcRadishPerSecond().toString();
    radishPriceNode.innerText = "Price/Radish: $" + currentRadishPrice.toString();
    radishCountNode.innerText = "Radishes: " + radishCount.toString();
};

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
    currentRadishPrice = baseRadishPrice * priceVariance;
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
    moneyCountNode.innerText = "Money: $" + moneyCount.toFixed(2);
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

// TODO: Update values based on login info rather than hardcoding everything
function generateCardArray() {
    let cardArray = [];
    cardArray.push(new marketCard("Farmer", "images/cat_farmer.png", "A drawing of a wide-eyed black cat wearing a tan farmer hat and overalls", 
                                  "This cat works hard to help you farm MEOWntains of radishes.", 
                                  INITIAL_FARMER_COST, INITIAL_FARMER_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Farm", "images/farm.png", "A drawing of a three-row radish plot", 
                                  "More fields of radishes! Along with increasing radish production, you also gain different radish varieties.", 
                                  INITIAL_FARM_COST, INITIAL_FARM_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Charlie", "images/charlie.png", "A drawing of a gold-capped conure, a parkeet with a red head, black beak, and green body.", 
                                  "(UNIQUE) Charlie's poop makes for great fertilizer! Your radishes can grow faster now if you feed Charlie almonds.", 
                                  CHARLIE_COST, CHARLIE_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Taffy", "images/taffy.png", "A drawing of an open candy wrapper with the text \'CAT-SAFE TAFFY\'. There is a tan piece of taffy sticking out.", 
                                  "Taffy for cats! Your farmers are motivated to work harder now. Speeds up farmer's radishes/second.", 
                                  INITIAL_TAFFY_COST, INITIAL_TAFFY_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Lobster", "images/lobster.png", "A drawing of a muted red lobster.", 
                                  "Mysteriously, having more lobsters in the water you use increases radish quality. Raise the price maximum by $0.20 for each lobster you have.", 
                                  INITIAL_LOBSTER_COST, INITIAL_LOBSTER_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Almond", "images/almond.png", "A drawing of an almond in its shell.", 
                                  "Charlie loves cracking open almonds! Eating an almond will make Charlie's poop even better.", 
                                  INITIAL_ALMOND_COST, INITIAL_ALMOND_COST, INITIAL_COUNT_ZERO));
    cardArray.push(new marketCard("Powell", "images/placeholder.png", "A black-and-white sketch of a wide-eyed black cat wearing a farmer hat and overalls.", 
                                  "(UNIQUE) Grants a boon based on how many farmers you have.", 
                                  POWELL_COST, POWELL_COST, INITIAL_COUNT_ZERO));

    return cardArray;
}

// TODO: update the descriptions to include the contribution to RPS (radishes/second)
function buyUpgrade(cardId, cardMap) {

    // Updating variables using struct
    const cardStruct = cardMap.get(cardId);
    let cost = cardStruct.cost;
    if (cost > moneyCount) {
        // TODO: Add visual feedback for not having enough money to buy an upgrade
        return;
    }
    moneyCount -= cost;
    moneyCountNode.innerText = "Money: $" + moneyCount.toFixed(2);

    cardStruct.count += 1;
    // Check if unique
    switch (cardId) {
        case "Charlie":
            hasCharlie = true;
            buyUniqueUpgrade("Charlie");
            return;
        case "Powell":
            hasPowell = true;
            buyUniqueUpgrade("Powell");
            return;
        default:
            break;
    }

    let newCost = calculateUpgradeCost(cardStruct.initialCost, cardStruct.count);
    cardStruct.cost = newCost;

    // Updating HTML using node
    const cardNode = document.getElementById(cardId);
    cardNode.querySelector(".upgrade_price").innerText = "Cost: $" + cardStruct.cost;
    cardNode.querySelector(".upgrade_count").innerText = "Number: " + cardStruct.count;


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
            break;
        case "Almond":
            almondCount += 1;
            break;
        default:
            break;
    }
    radishPerSecondNode.innerText = "Radishes/Sec: " + calcRadishPerSecond().toString();
}

function calculateUpgradeCost(initialCost, count) {
    return Math.floor(initialCost * EXPONENT_BASE ** count);
}

function buyUniqueUpgrade(name) {
    let upgrade = document.getElementById(name);
    upgrade.querySelector(".buy_button").className = "bought_button";
    upgrade.querySelector(".bought_button").innerText = "Bought";
    upgrade.querySelector(".upgrade_price").innerText = "";
    upgrade.querySelector(".upgrade_count").innerText = "";
    radishPerSecondNode.innerText = "Radishes/Sec: " + calcRadishPerSecond().toString();
}

function generateMarket() {
    const base = document.getElementById("Base");
    const marketNode = document.getElementById("market");

    let cardArray = generateCardArray();
    let cardMap = new Map();
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
        card.querySelector(".buy_button").addEventListener("click", function() {buyUpgrade(card.id, cardMap)});
    }

    return cardMap;
}


main();