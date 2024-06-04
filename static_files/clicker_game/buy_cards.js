function marketCard(name, image, alt, description, cost, count) {
    this.name = name;
    this.image = image;
    this.alt = alt;
    this.description = description;
    this.cost = cost;
    this.count = count;
}

// TODO: Update values based on login info rather than hardcoding everything
function generateCardArray() {
    let cardArray = [];
    cardArray.push(new marketCard("Farmer", "images/cat_farmer.png", "A drawing of a wide-eyed black cat wearing a farmer hat and overalls", "This cat works hard to help you farm MEOWntains of radishes.", 5, 0));
    cardArray.push(new marketCard("Farm", "images/farm.png", "A drawing of a three-row radish plot", "More fields of radishes! Along with increasing radish production, you also gain different radish varieties.", 5, 0));
    cardArray.push(new marketCard("Charlie", "images/placeholder.png", "A black-and-white sketch of a wide-eyed black cat wearing a farmer hat and overalls.", "Charlie's poop makes for great fertilizer! Your radishes will grow faster now.", 15, 0));
    cardArray.push(new marketCard("Taffy", "images/placeholder.png", "A black-and-white sketch of a wide-eyed black cat wearing a farmer hat and overalls.", "Taffy for cats! Your farmers are motivated to work harder now. Speeds up farmer's radishes/second.", 20, 0));
    cardArray.push(new marketCard("Lobster", "images/placeholder.png", "A black-and-white sketch of a wide-eyed black cat wearing a farmer hat and overalls.", "Mysteriously, having more lobsters in the water you use increases radish quality.", 5, 0));
    cardArray.push(new marketCard("Nut", "images/placeholder.png", "A black-and-white sketch of a wide-eyed black cat wearing a farmer hat and overalls.", "Charlie loves nuts! Eating a nut will make Charlie's poop even better.", 5, 0));
    cardArray.push(new marketCard("Aishiki-something", "images/placeholder.png", "A black-and-white sketch of a wide-eyed black cat wearing a farmer hat and overalls.", "Does something with radish quality", 5, 0));

    return cardArray;
}

function buyUpgrade(cardNode, cardMap) {
    let cardId = cardNode.id;
    // TODO: Change cost based on function that takes in starting cost and count
    cardMap.get(cardId).cost += 5;
    cardMap.get(cardId).count += 1;

    cardNode.querySelector(".upgrade_price").innerText = "Cost: $" + cardMap.get(cardId).cost;
    cardNode.querySelector(".upgrade_count").innerText = "Number: " + cardMap.get(cardId).count;
}

function generateMarket() {
    const base = document.getElementById("Base");
    const marketNode = document.getElementById("market");

    let cardArray = generateCardArray();
    let cardMap = new Map();
    for (let i = 0; i < cardArray.length; i++) {
        let clone = base.cloneNode(true);
        clone.setAttribute("id", cardArray[i].name);
        clone.querySelector(".upgrade_name").innerText = cardArray[i].name;
        clone.querySelector(".upgrade_img").src = cardArray[i].image;
        clone.querySelector(".upgrade_img").alt = cardArray[i].alt;
        clone.querySelector(".upgrade_info").innerText = cardArray[i].description;
        clone.querySelector(".upgrade_price").innerText = "Cost: $" + cardArray[i].cost;
        clone.querySelector(".upgrade_count").innerText = "Number: " + cardArray[i].count;
        marketNode.appendChild(clone);

        cardMap.set(cardArray[i].name, cardArray[i]);
    }

    let cards = document.getElementsByClassName("card");
    for (let card of cards) {
        card.querySelector(".buy_button").addEventListener("click", function() {buyUpgrade(card, cardMap)});
    }
}

// function clickRadish() {
//     document.getElementById("radishImg").src = "images/radish_clicked.png";
// }

// function generateClicker() {
//     document.getElementById("radishImg").addEventListener("click", clickRadish);
// }

const main = () => {
    console.log("index loaded!");
    //generateClicker();
    generateMarket();

};

main();