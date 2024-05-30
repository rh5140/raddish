//vars 
let radishCount = 0; //temp
let moneyCount = 0;
let farmerCount = 0;


//price fluctuation
let priceFluctuationTimer = 0;
let currentRadishPrice = 1;
let baseRadishPrice = 1;




const main = () => {
    console.log("index loaded!");
    setInterval(updateLoop, 1000); //1 second "tick" for now
    document.querySelector("#clickme").addEventListener("click", (event) => {
        event.currentTarget.innerText += " click me";
    })

    document.querySelector("#radishClick").addEventListener("click", onRadishClick);
    document.querySelector("#buyFarmer").addEventListener("click", buyFarmer);
    document.querySelector("#sellRadish").addEventListener("click", sellRadish.bind(null, 1));
    document.querySelector("#sellRadishTen").addEventListener("click", sellRadish.bind(null, 10));
    document.querySelector("#sellRadishHundred").addEventListener("click", sellRadish.bind(null, 100));
};


const onRadishClick = (event) => {
    radishCount += 1; //with more upgrades will increase by more than 1
    updateUI();
}

const buyFarmer = (event) => {
    if(moneyCount >= 10){
        moneyCount -= 10;
        farmerCount += 1;
        updateUI();
    }
}



//main event loop
const updateLoop = () => {
    radishCount += calcRadishPerSecond();

    priceFluctuationTimer += 1; //happens every 10 seconds for now
    if(priceFluctuationTimer >= 10){
        priceFluctuationTimer = 0;
        changeRadishPrice();
    }

    updateUI();
    
};

const updateUI = () => {
    var s = document.getElementById("radishCount");
    s.innerText = "Radishes: " + radishCount.toString();
    s = document.getElementById("farmerCount");
    s.innerText = "Farmers: " + farmerCount.toString();
    s = document.getElementById("radishPerSecond");
    s.innerText = "Radishes/Sec: " + calcRadishPerSecond().toString();


    //raddish price
    s = document.getElementById("radishPrice");
    s.innerText = "Price/Radish: " + currentRadishPrice.toString();

    //money
    s = document.getElementById("moneyCount");
    s.innerText = "Money: " + moneyCount;
}

const calcRadishPerSecond = () =>{
    return farmerCount * 1; //1 here as a placeholder - might be able to, say, upgrade farmers in the future.
}


const changeRadishPrice = () => {
    //25% price variance up or down at base - 0.75 to 1.75
    //maybe upgrade this later?
    //min max both inclusive
    let max = 125;
    let min = 75;
    var priceVariance = Math.floor(Math.random() * (max - min + 1)) + min;
    priceVariance = priceVariance / 100;
    currentRadishPrice = baseRadishPrice * priceVariance;

    //don't call updateUI, since it gets called in event loop
}

const sellRadish = (amount) =>{
    console.log(amount);
    if(radishCount >= amount){
        radishCount -= amount;
        moneyCount += amount * currentRadishPrice;
    }
    updateUI();
}


main();