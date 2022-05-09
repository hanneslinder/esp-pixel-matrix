var copyfiles = require('copyfiles');

const cssPath = "./dist/main.css";
const jsPath = "./dist/main.js";
const htmlPath = "./dist/index.html";
const destination = "../esp32/data";
 
copyfiles([cssPath, jsPath, htmlPath, destination], { up: 1 }, () => {
  console.log("Files copied to ", destination);
});