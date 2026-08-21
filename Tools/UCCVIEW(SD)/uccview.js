// Arquivo uccview.js
// Criado em 27/07/2016 por Acrisio
// UCC-VIEW do pangya

// Precisa de um File input e um Canvas

// Esses já estava no html
var canvas2 = document.createElement("canvas");
var file1 = document.createElement("input");

// Canvas Init
canvas2.id = "canvas2";
canvas2.width = 400;
canvas2.height = 400;
canvas2.style.border = "1px solid #000";

// File
file1.type = "file";

file1.addEventListener("change", showUcc, false);

// Add no HTML
document.body.appendChild(canvas2);
document.body.appendChild(file1);

function showUcc(e) {
	var file = e.target.files[0];

	if (file) {

		var reader = new FileReader();
		reader.onload = function(e) {
			printUcc(e.target.result, file);
		};
		reader.readAsBinaryString(file);
	}
};

function printUcc(content, file) {
	var ctx2 = canvas2.getContext("2d");

	var ax = Math.ceil(Math.sqrt(content.length/4));

	//console.log(content.length);
	var imgData;
	var state = false;

	// Cleam Canvas
	ctx2.fillStyle = "white";
	ctx2.fillRect(0, 0, 400, 400);


	if (file.name.toLowerCase().includes("front") || file.name.toLowerCase().includes("back")) {
		// Parts 		 128x256
		// Parts Vestido 256x256
		var parts_width = 128;

		if (content.length > 100000) {
			parts_width = 256;
		}

		imgData = ctx2.createImageData(parts_width, 256);
		var j = 0;

		for (var i = 0; i < imgData.data.length; i += 4) {
			if ((j + 3) < content.length) {
				imgData.data[i+0] = content.charCodeAt(j+2);
				imgData.data[i+1] = content.charCodeAt(j+1);
				imgData.data[i+2] = content.charCodeAt(j+0);
				imgData.data[i+3] = 0xFF; // Alpha

				j += 3;
			}else {
				imgData.data[i] = Math.floor(Math.random() * 255);
			}
		}

		state = true;
	}else if (file.name.toLowerCase().includes("icon")) {
		// Icon 64x84
		imgData = ctx2.createImageData(64, 84);

		for (var i = 0; i < imgData.data.length; i += 4) {
			if ((i + 4) < content.length) {
				imgData.data[i+0] = content.charCodeAt(i+2);
				imgData.data[i+1] = content.charCodeAt(i+1);
				imgData.data[i+2] = content.charCodeAt(i+0);
				imgData.data[i+3] = content.charCodeAt(i+3);	// Alpha
			}else {
				imgData.data[i] = Math.floor(Math.random() * 255);
			}
		}

		state = true;
	}

	if (state) {
		ctx2.putImageData(imgData, 10, 10, 0, 0, 300, 300);
	}
};