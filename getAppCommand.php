<?php

$username="trepico9_PDAE";//change username 
$password="PDAE123"; //change password
$host="localhost";
$db_name="trepico9_PDAE"; //change databasename
$conn = mysqli_connect($host,$username,$password,$db_name);

if ($conn->connect_error) {
	die("Connection failed: " . $conn->connect_error);
} 
 
$sql = "select ESP from SistemasEmbebidos";

$result = $conn->query($sql);
 
if ($result->num_rows >0) {
 
	 while($row = $result->fetch_assoc()) {

	 	echo $row['ESP'];

 	}
 
} else {
	echo "!";
}

$conn->close();

?>