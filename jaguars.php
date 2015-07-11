<html>
<body>
<h1> Jayanagar Jaguars Fees Payment</h1>

<?php
// make mysql query, read all the names. walk thru the list and prepare <option> tags

$jag_db = new jaguars_db;

$nfc = $_GET['nfc'];
$fees = $_GET['fees'];
$phone = $_GET['phone'];
if ($nfc) {
	// if there's an NFC tag in URL
	$jag_id = $jag_db->get_id_from_tag ($nfc);
	if ($jag_id) {
		$jag_name = $jag_db->get_name_from_id ($jag_id);
		if ($jag_name) {
			printf ("Welcome %s (%s)", $jag_name, $jag_id);
			echo "<br>";
			printf ("Your last fees was paid on %s for Rs.%s\n", 
				$jag_db->get_last_fees_paid_on ($jag_id),
				$jag_db->get_last_fees_paid_amount ($jag_id));
			echo "<br>";
		}
	}
} else if ($phone) {
	// look up phone number from profile and get the jag_id
	$jag_id = $jag_db->get_id_from_phone ($phone);
	if ($jag_id) {
		$jag_name = $jag_db->get_name_from_id ($jag_id);
		if ($jag_name) {
			printf ("Welcome %s (%s)", $jag_name, $jag_id);
			echo "<br>";
			echo "<input type=\"hidden\" name=\"jag_id\" value=\"$jag_id\")";
			echo "<br>";
			printf ("Your last fees was paid on %s for Rs.%s\n", 
				$jag_db->get_last_fees_paid_on ($jag_id),
				$jag_db->get_last_fees_paid_amount ($jag_id));
			echo "<br>";
		}
	}
} else if ($fees) {
	// else if fees is getting paid
	$jag_id = $_GET['jag_id'];
	if ($jag_id) {
		$event = $_GET['event'];
		if ($event) {
			$months = event_to_months ($event);
			$jag_db->submit_fees ($jag_id, $fees, $months);
		}
	}
} else {
	// display all registered jaguars
	$list = $jag_db->get_all_runners_names ();
	echo '<form action="#"> <select multiple name="jag_id">';
	foreach ($list as $id => $name) {
		printf ("<option value=\"%s\">%s</option>", $id, $name);
	}
	echo '</select>';
}

echo <<<_REGISTERED_JAGS
  <br> <br>
  Total Fees Paid:<br>
  <input type="number" name="fees">

  <br> <br>
<fieldset>
    <legend>Select Event or Months</legend>
    <input type="radio" name="event" id="ryfm" value="ryfm" /><label for="ryfm">RYFM</label>
    <input type="radio" name="event" id="scmm" value="scmm" /><label for="scmm">SCMM</label>
    <input type="radio" name="event" id="tcs10k" value="tcs10k" /><label for="scmm">TCS-10K</label><br>
    <br>
    <input type="radio" name="jan" id="jan" value="jan"  /><label for="jan">January</label>
    <input type="radio" name="feb" id="feb" value="feb"  /><label for="feb">February</label>
    <input type="radio" name="mar" id="mar" value="mar"  /><label for="mar">March</label><br>
    <input type="radio" name="apr" id="apr" value="apr"  /><label for="apr">April</label>
    <input type="radio" name="may" id="may" value="may"  /><label for="may">May</label>
    <input type="radio" name="jun" id="jun" value="jun"  /><label for="jun">June</label><br>
    <input type="radio" name="jul" id="jul" value="jul"  /><label for="jul">July</label>
    <input type="radio" name="aug" id="aug" value="aug"  /><label for="aug">Aug</label>
    <input type="radio" name="sep" id="sep" value="sep"  /><label for="sep">Sept</label><br>
    <input type="radio" name="oct" id="oct" value="oct"  /><label for="oct">October</label>
    <input type="radio" name="nov" id="nov" value="nov"  /><label for="nov">November</label>
    <input type="radio" name="dec" id="dec" value="dec"  /><label for="dec">Dececember</label><br>
</fieldset>

  <br>
  <input type="submit" value="Submit">
</form>

_REGISTERED_JAGS;

class jaguars_db {

	private $link = NULL;

	function __construct ()
	{
		$this->link = mysql_connect('localhost', 'pi', 'raspberrypi');
		if (!$this->link) {
		    echo 'Could not connect to mysql';
		    exit;
		}

		if (!mysql_select_db('jaguars', $this->link)) {
		    echo 'Could not select database';
		    exit;
		}

	}

	private function __get_ret_from_idx ($table, $idx_col, $idx_val, $ret_col) {
		$sql    = "SELECT $ret_col FROM $table WHERE $idx_col LIKE '$idx_val'";
		$result = mysql_query($sql, $this->link);
		if (!$result) {
			echo "DB Error, could not query the database\n";
			echo 'MySQL Error: ' . mysql_error();
			return NULL;
		}
		$ret = NULL;
		if ($row = mysql_fetch_array($result, MYSQL_NUM)) {
			$ret = $row[0];
		}
		mysql_free_result($result);
		return $ret;
	}

	function get_id_from_phone ($phone) {
		return $this->__get_ret_from_idx ('profile', 'phone', $phone, 'userid');
	}

	function get_id_from_tag ($nfc) {
		// serial is the column for NFC tag
		return $this->__get_ret_from_idx ('runners', 'serial', $nfc, 'userid');
	}

	function get_name_from_id ($id) {
		return $this->__get_ret_from_idx ('profile', 'userid', $id, 'name');
	}

	function get_last_fees_paid_on ($id) {
		return $this->__get_ret_from_idx ('fees', 'userid', $id, 'paid_on');
	}

	function get_last_fees_paid_amount ($id) {
		return $this->__get_ret_from_idx ('fees', 'userid', $id, 'amount');
	}

	function get_all_runners_names () {

		// display all users as selection
		$sql    = "SELECT userid FROM runners";
		$result = mysql_query($sql, $this->link);
		if (!$result) {
			echo "DB Error, could not query the database\n";
			echo 'MySQL Error: ' . mysql_error();
			return 0;
		}

		while ($row = mysql_fetch_array($result, MYSQL_NUM)) {
			$list[$row[0]] = $this->get_name_from_id ($row[0]);
		}
		mysql_free_result($result);
		print_r ($list);
		return $list;
	}

	function submit_fees ($jag_id, $fees, $months) {

		// display all users as selection
		$months_bm = months_bitmap ($months);
		$sql    = "INSERT INTO fees values ('$jag_id', NOW(), $fees, 2015, $months_bm)";
		echo $sql;
		$result = mysql_query($sql, $this->link);
		if (!$result) {
			echo "DB Error, could not query the database\n";
			echo 'MySQL Error: ' . mysql_error();
			return 0;
		}

		while ($row = mysql_fetch_array($result, MYSQL_NUM)) {
			$list[$row[0]] = $this->get_name_from_id ($row[0]);
		}
		mysql_free_result($result);
		print_r ($list);
		return $list;
	}

}
	function event_to_months ($event) {
		$event_months = array (
			'ryfm' => array ("jul", "aug", "sep", "oct"),
			'scmm' => array ("nov", "dec", "jan"),
			'tcs10k' => array ("mar", "apr", "may"));
		return $event_months[$event];
	}

	function months_bitmap ($months) {
		$months_bitmap = array (
			'jan' => 0b100000000000, 'feb' => 0b010000000000, 'mar' => 0b010000000000,
			'apr' => 0b000100000000, 'may' => 0b000010000000, 'jun' => 0b000001000000,
			'jul' => 0b000000100000, 'aug' => 0b000000010000, 'sep' => 0b000000001000,
			'oct' => 0b000000000100, 'nov' => 0b000000000010, 'dec' => 0b000000000001);
		$ret_bitmap = 0;
		foreach ($months as $item) {
			$ret_bitmap += $months_bitmap[$item];
		}
		return $ret_bitmap;
	}

?>

</body>
</html>
