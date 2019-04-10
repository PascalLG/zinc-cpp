<?php
	function dump($var) {
		if (count($var) > 0) {
			echo '<table>';
			echo '<tr><th>Key</th><th>Value</th></tr>';
			foreach($var as $key => $value) {
				echo '<tr><td>', $key, '</td><td>', $value, '</td></tr>';
			}
			echo '</table>';
		} else {
			echo '<p>Empty.</p>';
		}
	}
?>
<!doctype html>
<html lang="en">
<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
	<link rel="stylesheet" type="text/css" href="style.css"/>
	<title>Tests</title>
</head>
<body>
	<div id="header">Simple form</div>
	<div id="content">
		<h1>SERVER variables</h1>
		<?php dump($_SERVER); ?>
		<h1>GET variables</h1>
		<?php dump($_GET); ?>
		<h1>POST variables</h1>
		<?php dump($_POST); ?>
		<p><a href="index.html">Back to home page</a></p>
	</div>
</body>
</html>
