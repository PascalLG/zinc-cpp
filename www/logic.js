function update(extension) {
	document.getElementById('simple_form').action = 'simple.' + extension;
	document.getElementById('upload_form').action = 'upload.' + extension;
}

window.onload = function() {
	document.getElementById('radio_php').addEventListener('click', function() { update('php'); });
	document.getElementById('radio_python').addEventListener('click', function() { update('py'); });
};
