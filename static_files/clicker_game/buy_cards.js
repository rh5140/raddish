let upgradePrice = 5;
let upgradeCount = 0;
let card = document.getElementsByClassName("card")[0];

const main = () => {
    console.log("index loaded!");
    card.querySelector(".buy_button").addEventListener("click", buyUpgrade);
};

const buyUpgrade = () => {
    upgradePrice += 5;
    upgradeCount += 1;
    card.querySelector(".upgrade_price").innerText = "Cost: $" + upgradePrice.toString();
    card.querySelector(".upgrade_count").innerText = "Number: " + upgradeCount.toString();
}

main();