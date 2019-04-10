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
	<div id="header">Upload file</div>
	<div id="content">
		<h1>SERVER variables</h1>
		<?php dump($_SERVER); ?>
		<h1>FILES variables</h1>
		<?php dump($_FILES['filename']); ?>
		<h1>File content</h1>
		<pre><?php 
			$fp = fopen($_FILES['filename']['tmp_name'], 'r');
			$count = 0;
			while (!feof($fp) && $count < 100) {
				$line = fread($fp, 16);
				$length = strlen($line);
				for ($i = 0; $i < $length; $i++) {
					echo bin2hex($line[$i]), ' ';
				}
				echo '   ';
				for ($i = 0; $i < $length; $i++) {
					$ch = $line[$i];
					if (ctype_print($ch)) {
						echo $ch, '  ';
					} else if ($ch == "\n") {
						echo '\n ';
					} else if ($ch == "\r") {
						echo '\r ';
					} else if ($ch == "\t") {
						echo '\t ';
					}  else {
						echo '   ';
					}
				}
				echo "\r\n";
				$count++;
			}
			fclose($fp);
		?></pre>
		<p><a href="index.html">Back to home page</a></p>
	</div>
</body>
</html>
