const main = () => {
    console.log("index loaded!");
    document.querySelector("#clickme").addEventListener("click", (event) => {
        event.currentTarget.innerText += " click me";
    })
};
main();