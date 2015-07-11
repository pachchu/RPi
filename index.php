<html><body><h1>GPIO Web Utility!</h1>
<p>This web utility sets the GPIO mode and values.</p>

<form action="gpio_readall.php">
    <input type="submit" value="GPIO Read All">
</form>

<form action="gpio_mode.php">
    <input type="submit" value="Mode">
</form>

<form action="gpio_write.php">
    <input type="submit" value="Write">
</form>

<form action="#">
  <select name="shut">
    <option value="halt">Halt</option>
    <option value="reboot">Reboot</option>
    <option value="poweroff">Poweroff</option>
  </select>

  <input type="submit" value="Submit">
</form>

<?php
$shut = $_GET['shut'];
system('sudo '.' '.escapeshellarg($shut));
?>

</body></html>
