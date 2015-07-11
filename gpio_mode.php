<html>
<body>
<h1>GPIO Mode</h1>
<p>Sets the GPIO mode to IN or OUT</p>

<form action="#">
  <select multiple autofocus name="gpio">
    <option value="7">7</option>
    <option value="0">0</option>
    <option value="1">1</option>
    <option value="2">2</option>
    <option value="3">3</option>
    <option value="4">4</option>
    <option value="5">5</option>
    <option value="6">6</option>
    <option value="21">21</option>
    <option value="22">22</option>
    <option value="23">23</option>
    <option value="24">24</option>
    <option value="25">25</option>
    <option value="26">26</option>
    <option value="27">27</option>
    <option value="28">28</option>
    <option value="29">29</option>
  </select>

  <select name="val">
    <option value="out">out</option>
    <option value="in">in</option>
  </select>

  <input type="submit" value="Submit">
</form>

<?php
$gpio = $_GET['gpio'];
$val = $_GET['val'];
system('gpio mode '.' '.escapeshellarg($gpio).' '.escapeshellarg($val));
?>

<form action="gpio_readall.php">
    <input type="submit" value="GPIO Read All">
</form>

<form action="index.php">
    <input type="submit" value="GPIO Home">
</form>

</body>
</html>
