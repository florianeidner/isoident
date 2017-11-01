<?php

$data = $_POST['xml'];

//set mode of file to writable.

chmod("/home/florian/isoident/isoident.xml",0777);

$f = fopen("/home/florian/isoident/isoident.xml", "w+") or die("fopen failed");

fwrite($f, $data);

fflush($f);

fclose($f);

?>