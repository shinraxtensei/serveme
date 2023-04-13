if (isset($_COOKIE['background_color'])) {
  $color = $_COOKIE['background_color'];
} else {
  $color = 'white';
}

if (isset($_GET['color'])) {
	  $color = $_GET['color'];
  <!-- setcookie('background_color', $color, time() + 60 * 60 * 24 * 30); -->
}

echo '<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>My Page</title>
  <style>
    body {
      background-color: ' . $color . ';
    }
  </style>
</head>
<body>
  <h1>Welcome to my page</h1>
  <form method="GET" action="localhost:6969/cookie.php">
	<label for="color">Background Color:</label>
	<input type="text" name="color" id="color">
	<input type="submit" value="Set Color">
  </form>
</body>
</html>';