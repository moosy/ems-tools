#!/usr/bin/php 
<?php
# Note: the following PHP-Modules are needed: posix, pcntl, shmop
error_reporting (0);
set_time_limit (0);
ob_implicit_flush ();
$shm_key = ftok(__FILE__, 't');

$ems_address = '127.0.0.1';
$ems_service_port = 7778;

$address = '127.0.0.1';
$port = 7779;

# Open Server Socket for incoming connections

if (($srv_sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP)) === false) {
    echo "Creating of incoming socket failed: " . socket_strerror(socket_last_error()) . "\n";
}
if (socket_bind($srv_sock, $address, $port) === false) {
    echo "Bind to incoming socket failed: " . socket_strerror(socket_last_error($srv_sock)) . "\n";
}
if (socket_listen($srv_sock, 5) === false) {
    echo "Listening to incoming socket failed: " . socket_strerror(socket_last_error($srv_sock)) . "\n";
}

# Open Socket to EMS-Info-Server

$ems_socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

if ($ems_socket === false) {
    die ("Creation of socket to collectord failed: " . socket_strerror(socket_last_error()) . "\n");
  }

$result = socket_connect($ems_socket, $ems_address, $ems_service_port);
if ($result === false) {
  die ("Connect to collectord failed ($result): " . socket_strerror(socket_last_error($ems_socket)) . "\n");
}

$pid = pcntl_fork();
if ($pid == -1) {
  die('fork of father child failed\n');
} else if ($pid) {
   // === grandfather thread (data collector) ===
   $smid=shmop_open($shm_key,"c",0644,1024000);  // 1MB of shared mem should suffice
   $data = array();
   $ds = serialize($data);
   shmop_write($smid,$ds,0);
              
   while (1){
     while ($in = socket_read($ems_socket, 2048)) {
       $tstamp = mktime();
       $ds =shmop_read($smid,0,1024000);
       $data = unserialize($ds);
       $ina = explode("\n",$in);
       foreach ($ina as $l){
         $la = explode(" ",trim($l));
         $key = trim(array_shift($la));
         if (count($la)>1) $key .= " ".trim(array_shift($la));
         $value = trim(implode(" ",$la));       
         if (strtok($key," ") == "systemtime"){
           $value = strtok(" ")." ".$value;
           $key = "systemtime";
         }
         if ((trim($key)!="") && (trim($value)!="")) $data[$key] = array("value"=>$value, "time"=>$tstamp);
       }
       ksort($data);
       $ds = serialize($data);
       shmop_write($smid,$ds,0);
     }
    socket_shutdown($ems_socket);
    socket_close($ems_socket);
    # Read failed, probably the collectord died.  Wait and try reconnecting.
    sleep(2);
    $ems_socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

    if ($ems_socket === false) {
      die ("Creation of socket to collectord failed: " . socket_strerror(socket_last_error()) . "\n");
    }
    $result = socket_connect($ems_socket, $ems_address, $ems_service_port);
    if ($result === false) {
      die ("Connect to collectord failed ($result): " . socket_strerror(socket_last_error($ems_socket)) . "\n");
    }
   }

} else {
    // === father thread, creates 4 child listeners and takes care of them ===
    $childs = 4;
    while (1) {
      $pid = pcntl_fork();
      if ($pid == -1) {
        die('fork of listener child failed\n');
      } else if ($pid) {
         // === we are daddy, lets look after the kids ===
         $childs--;
         if ($childs <= 0)  pcntl_wait($status); 
      } else {

        // === child listener thread ===
        if (($msgsock = socket_accept($srv_sock)) === false) {
          echo "Listener: accept() failed: " . socket_strerror(socket_last_error($srv_sock)) . "\n";
          break;
        }
        print ("Connection accepted ".getmypid()."\n");
        $smid=shmop_open($shm_key,"c",0644,1024000); 

        while (1) {
            if (false === ($buf = socket_read ($msgsock, 2048, PHP_NORMAL_READ))) {
                echo "socket_read() fehlgeschlagen: Grund: " . socket_strerror(socket_last_error($msgsock)) . "\n";
                break;
            }
            if (!$buf = trim ($buf)) {
                continue;
            }
            if ($buf == 'quit') {
                break;
            }
            if ($buf == 'flush') {
                $data = array();
                $ds = serialize($data);
                shmop_write($smid,$ds,0);
                continue;
            }
            if ($buf == 'getdata') {
              $ds =shmop_read($smid,0,1024000);
              $data = unserialize($ds);
              $buf = "";
              foreach ($data as $k => $v){
                $buf .= "$k = $v[value] | $v[time]\n";
              }
            }
            $talkback = $buf."OK\n";
            socket_write ($msgsock, $talkback, strlen ($talkback));
        }
        print ("Connection closed ".getmypid()."\n");
        socket_shutdown($msgsock);
        socket_close ($msgsock);
        posix_kill(getmypid(),SIGTERM);
      } // child
    } // father
} // grandfather
socket_close ($srv_sock);
?>