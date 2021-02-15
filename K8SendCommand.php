<?php

$username="trepico9_PDAE";//change username 
$password="PDAE123"; //change password
$host="localhost";
$db_name="trepico9_PDAE"; //change databasename

$conn = mysqli_connect($host,$username,$password,$db_name);

$command = $_GET['command'];

if ($conn->connect_error) {
	die("Connection failed: " . $conn->connect_error);
} 
 
$sql = "UPDATE `SistemasEmbebidos` SET `ESP` = '$command' WHERE `SistemasEmbebidos`.`ID` = 1;";
$results = mysqli_query($conn, $sql);

echo $command;

$conn->close();

?>