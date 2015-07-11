<html><body><h1>GPIO Web Utility!</h1>

<?php
echo '<pre>';
system('gpio '.escapeshellarg($cmd).' '.escapeshellarg($gpio).' '.escapeshellarg($val));
system('gpio readall');
echo '</pre>'
?>

</body></html>
