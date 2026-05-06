<?php
require("/emsincludes1/config.php");
require_once("/emsincludes1/emschoosers.inc");

$daystr = array("0" => "Montag",
		"1" => "Dienstag",
		"2" => "Mittwoch",
		"3" => "Donnerstag",
		"4" => "Freitag",
		"5" => "Samstag",
		"6" => "Sonntag");
$daystr2 = array("monday" => "Montag",
		 "tuesday" => "Dienstag",
		 "wednesday" => "Mittwoch",
	 	 "thursday" => "Donnerstag",
		 "friday" => "Freitag",
		 "saturday" => "Samstag",
		 "sunday" => "Sonntag");

function flush_buffers(){ 
     ob_flush(); 
     flush(); 
}

function open_ems(){
  global $ems_socket;
  $address = "127.0.0.1";
  $service_port = 7777;
  if (!isset($ems_socket)){
    $ems_socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    if ($ems_socket === false) {
      die("FATAL: Keine Verbindung zum EMS-Bus!\nGrund: " . socket_strerror(socket_last_error()) . "\n");
    }
    // Connect-Timeout via non-blocking
    socket_set_nonblock($ems_socket);
    $result = @socket_connect($ems_socket, $address, $service_port);
    $write = [$ems_socket];
    $read = $except = null;
    if (socket_select($read, $write, $except, 5) < 1) {
      die("FATAL: Keine Verbindung zum EMS-Bus! (connect timeout)\n");
    }
    socket_set_block($ems_socket);
    // 3 Sekunden Read-Timeout (sec + usec)
    socket_set_option($ems_socket, SOL_SOCKET, SO_RCVTIMEO, ['sec' => 3, 'usec' => 0]);
    // 3 Sekunden Send-Timeout
    socket_set_option($ems_socket, SOL_SOCKET, SO_SNDTIMEO, ['sec' => 3, 'usec' => 0]);
  }
  return true;
}

function close_ems(){
  global $ems_socket;
  if (isset($ems_socket)){
    // Timeout ist bereits gesetzt, daher hängt socket_read maximal 3s
    socket_shutdown($ems_socket, 1);
    // Maximale Iterationen begrenzen damit wir nie ewig loopen
    $maxiter = 100;
    while ($maxiter-- > 0 && socket_read($ems_socket, 256));
    socket_close($ems_socket);
    unset($ems_socket);
  }
}

function idiv($a,$b){
  return (($a - ($a % $b)) / $b);
}


function doEmsCommand($in){
  global $ems_socket;
  open_ems();
  $in = $in."\n";
  do {
    // Kein blindes garbage-lesen mehr!
    socket_write($ems_socket, $in, strlen($in));
    $res = "";
    $tout = 0;
    while (1) {
          usleep(5000); // 5ms wirklich warten statt busy-poll
          $out = socket_read($ems_socket, 2048);
          $tout++;
          if (strlen($out)>0) $tout=0;
          if ($tout > 200) break;  // 200 * 5ms = 1s Timeout
          $res .= $out;
          if (strpos(" ".$res,"ERRBUSY")) {
            usleep(200000); // länger warten bei busy
            break;
          }
          if (strpos(" ".$res,"OK"))  break 2;
          if (strpos(" ".$res,"ERRARGS")) break 2;
          if (strpos(" ".$res,"ERRCMD")) break 2;
    }
  } while (strpos(" ".$res,"ERRBUSY"));
  $resa = explode("\n",$res);
  if (!trim(end($resa))) array_pop($resa);
  return $resa;
}


function doEmsDataCommand($in = "", $wait = false) {
    global $ems_socket, $reqdone;

    if (!isset($reqdone)) $reqdone = array();

    $in = trim($in);

    $checks = array(
        "hk1 requestdata" => "hk1 partyhours",
        "hk2 requestdata" => "hk2 partyhours",
        "hk3 requestdata" => "hk3 partyhours",
        "hk4 requestdata" => "hk4 partyhours",
        "ww requestdata"  => "ww desinfectionday",
        "rc requestdata"  => "rc buildingtype",
        "uba requestdata" => "heaterpump followupminutes",
    );

    $check  = "";
    $oldest = 0;

    if ($in) {
        $oldest = time();

        if ($wait || !isset($reqdone[$in])) {
            $check = isset($checks[$in]) ? $checks[$in] : "";
            doEmsCommand($in);
            $reqdone[$in] = time(); // Timestamp statt bool – wann wurde zuletzt angefordert?
        } else {
            // Daten sind älter als 60s? Trotzdem neu anfordern
            if ((time() - $reqdone[$in]) > 60) {
                $check = isset($checks[$in]) ? $checks[$in] : "";
                doEmsCommand($in);
                $reqdone[$in] = time();
            }
        }
    }

    // Kurz warten damit der Collector die requestdata-Antwort verarbeiten kann
    // bevor wir den Cache pollen
    if ($check) usleep(300000); // 300ms head-start

    $maxTries = 20;
    $res = array();

    for ($tout2 = 0; $tout2 < $maxTries; $tout2++) {

        usleep(150000); // 150ms zwischen Cache-Fetches – weniger aggressiv
        $res = doEmsCommand("cache fetch");

        if (!$check) break; // kein Check nötig, einmal reicht

        $found = false;
        foreach ($res as $r) {
            if (strpos($r, $check) !== false) {
                $va      = explode("|", $r);
                $valtime = isset($va[1]) ? (int) trim($va[1]) : 0;
                if ($valtime > $oldest) {
                    $found = true;
                    break;
                }
            }
        }

        if ($found) break;

        if ($tout2 == $maxTries - 1) {
            error_log("doEmsDataCommand: Timeout warten auf '$check' nach '$in'");
        }
    }

    // Trailing OK und Leerzeilen entfernen – sauber ohne Pointer-Trickserei
    while (!empty($res) && trim(end($res)) === "") array_pop($res);
    while (!empty($res) && trim(end($res)) === "OK") array_pop($res);
    while (!empty($res) && trim(end($res)) === "") array_pop($res);

    return $res;
}

function parseTime($in){
  //  Convert MO 12:34 in minutes since Monday, 0:00

  $si = explode(" ",$in);
  $day=99;
  if ($si[0] == "monday") $day=0;
  if ($si[0] == "tuesday") $day=1;
  if ($si[0] == "wednesday") $day=2;
  if ($si[0] == "thursday") $day=3;
  if ($si[0] == "friday") $day=4;
  if ($si[0] == "saturday") $day=5;
  if ($si[0] == "sunday") $day=6;
  $sj = explode(":",$si[1]);
  if ($day==99) return false;
  return ($day*24*60 + $sj[0]*60 + $sj[1] - ($sj[1] % 10) ); // 10 Minuten Raster!
}

function day2str($day){
  $days = "monday";
  if ($day == 0) $days="monday";
  if ($day == 1) $days="tuesday";
  if ($day == 2) $days="wednesday";
  if ($day == 3) $days="thursday";
  if ($day == 4) $days="friday";
  if ($day == 5) $days="saturday";
  if ($day == 6) $days="sunday";
  return $days;

}

function synthTime($in){
  // converts minutes since Mon 00:00 to MO 12:34
  $day = idiv($in,24*60);
  return (day2str($day)." ".hm2str( ($in % (24*60))*60 ));
  
}

function time2dayhm($in){
  // converts minutes since Mon 00:00 to 0 <seconds> <15mins>
  $day = idiv($in,24*60);
  
  return array($day, ($in % (24*60))*60, idiv($in % (24*60),15) );
  
}

function hm2str($in){
  if ($in === false) return "";
  return date("H:i",$in-60*60);
}


function parseSchedule($in){
#  var_dump($in);
  $sch = array();
  foreach ($in as $l){
    if (trim($l) == "OK") continue;
    $si = explode(" ",$l);
    $tgt = $l[0];
    if (count($si)<6) continue;
    $tm = $si[4] * 24*60 + $si[5]*15;  // Minutes since Mon 00:00
    $cmd = trim($si[3]);
    $sch[$tm] = $tgt.$cmd;
  }
  ksort($sch);
  $sch2 = array();
  $old = "INVALID";
  foreach($sch as $k => $v){
    if ($old !== $v){
      $sch2[$k] = $v;
      $old=$v;
    }
  }
  return $sch2;
}

function printSchedule($sch){
  foreach ($sch as $s => $cmd){
    print(synthTime($s).
      " ".$cmd."\n");
  }
}


function schcmd2int($cmd){
  
  $val =  (substr($cmd,1,1)-0);

  return $val;
}


function schedule2graph($sch,$outfn){
  global $daystr;
  global $graphpath;
#  var_dump($sch);

  $amp=0.2; // Amplitude in Graph

  $wechsel = array();
  foreach($sch as $in => $v){
    $i = time2dayhm($in);
    $day = $i[0];
    $wechsel[$day] = schcmd2int($v);
  }

  $doffset = mktime(0,0,0,1,3,2000); # Align Monday
  $fd=array();
  $fname=array();

  $wechsel = array();
  foreach($sch as $in => $v){
    $i = time2dayhm($in);
    $day = $i[0];
    $wechsel[$day+1] = schcmd2int($v);
  }

  for ($k=1;$k<=7;$k++) if (isset($wechsel[$k])) $wechsel[0]=$wechsel[$k];

  for ($k=1;$k<=7;$k++) if (!isset($wechsel[$k])) $wechsel[$k]=$wechsel[$k-1];

  $pval = array();

  for ($day=0;$day<=6;$day++){
    $fname[$day] = "/tmp/schedule-".time()."-".$day.".dat";
    $fd[$day] = fopen($fname[$day],"w");
    $v = $wechsel[$day];
    $pval[$day] = $v;
    $plotv=(6-$day)+$amp*$v;
    
    fwrite($fd[$day],"2000-01-03 00:00:00 ".($plotv)."\n");
  }
  
  foreach($sch as $in => $v){
    $i = time2dayhm($in);
    $day = $i[0];
    $hm  = $i[1];
    $newval = schcmd2int($v);
    
    $plotv1 = (6-$day) + $amp * $pval[$day] ;
    $plotv2 = (6-$day) + $amp * $newval; 

    $pval[$day] = $newval;    
    fwrite($fd[$day],date("Y-m-d H:i:s",$hm + $doffset)." ".($plotv1)."\n");
    fwrite($fd[$day],date("Y-m-d H:i:s",$hm + $doffset)." ".($plotv2)."\n");
  }  

  for ($day=0;$day<=6;$day++){
    $v = $wechsel[$day+1];
    $plotv=(6-$day)+$amp*$v;

    fwrite($fd[$day],"2000-01-03 23:59:30 ".($plotv)."\n");
    fclose($fd[$day]);
  }


    $p = popen("gnuplot", "w");
    fwrite($p,"set terminal png font 'arial' 12 size 800, 450\n");
     fwrite($p,"set grid lc rgb '#aaaaaa' lt 1 lw 0,5\n");
     fwrite($p,"set title 'Schaltzeiten'\n");
     fwrite($p,"set xdata time\n");
     fwrite($p,"set xlabel 'Zeit'\n");
     fwrite($p,"set ylabel 'Reduziert / Normal'\n");
     fwrite($p,"set timefmt '%Y-%m-%d %H:%M:%S'\n");
     fwrite($p,"set format x '%H:%M'\n");
     fwrite($p,"set xtics 3600 rotate by -45\n");
     fwrite($p,"set ytics autofreq\n");
     fwrite($p,"set key outside\n");

     fwrite($p,"set output '$graphpath$outfn'\n");
     fwrite($p,"plot");
     $first=true;
     foreach ($fname as $fn){
       if (!$first) fwrite($p," , ");
       $first = false;
       $days = $daystr[substr($fn,-5,1)];
       fwrite($p," '".$fn."' using 1:3 with lines lw 6 title '".$days."' ");
     }
     fwrite($p,"\n");

     pclose($p);

     foreach ($fname as $fn) unlink($fn);

}

function formatval($sinf,$day,$on,$i,$onpadding){
  if ($onpadding){
    if ($i==0) return "";
    $i--;
  }
  $res=hm2str($sinf[$day][$on][$i]);
  return $res;
}


function schedule2html($sch,$src){
  global $daystr,$html_cols;

  $src = strtoupper($src);
  
  $kind = substr($src,0,1);
  if ($kind == "W") $kind = substr($src,-1);

  $sinf = array();
  foreach($sch as $k => $v){
    if (!isset($sinf[$day])) $sinf[$day] = array();
    $dhm = time2dayhm($k);
    $day = $dhm[0];
    $t15 = $dhm[2];
    $cmd = $v;

    $sinf[$day][] = array("t15"=>$t15,"cmd"=>$cmd);
  }

  for($day=0;$day<=6;$day++){
  
    print("<tr>");
    print("<td bgcolor=#cccccc><b>".$daystr[$day]."</b>");
    
    readdatechooser("rday-".$day, $day);
    
    print("</td>");

    print("<td bgcolor=#dddddd><h3>".chr($day+65)."</h3></td>");

    for ($i=0;$i<$html_cols;$i++){
    
      print("<td bgcolor=#dddddd>");
//      var_dump($sinf[$day][$i]);
      if (isset($sinf[$day][$i])){
        
        hourchooser("sct-$day-$i",$sinf[$day][$i]["t15"]/4, true);
        $kind = substr($sinf[$day][$i]["cmd"],0,1);
        switch ($kind){
          case "W":
            schwwmodechooser("scm-$day-$i",$sinf[$day][$i]["cmd"]);
            break;
          case "H":
            schhkmodechooser("scm-$day-$i",$sinf[$day][$i]["cmd"]);
            break;
          case "Z":
            schzpmodechooser("scm-$day-$i",$sinf[$day][$i]["cmd"]);
            break;
        }
      } else {
      
        hourchooser("sct-$day-$i",255,true);
        switch ($kind){
          case "W":
            schwwmodechooser("scm-$day-$i",0);
            break;
          case "H":
            schhkmodechooser("scm-$day-$i",0);
            break;
          case "Z":
            schzpmodechooser("scm-$day-$i",0);
            break;
        }
      
      }
      print("</td>");
    }
    print("</tr>\n");
  }
}


function html2sched($post,$ids){
  global $html_cols;
  $schstr = "";
  $ids = strtoupper($ids);
  $id = substr($ids,0,1);
  if (substr($ids,2,1) == "Z") $id = "Z";
  
  $idx = 1;
  for($day=0;$day<=6;$day++){
    for ($i=0;$i<$html_cols;$i++){
      $rday = trim($post["rday-".$day]);
      if (isset($post["sct-".$rday."-".$i]) ) {
        $tm = trim($post["sct-".$rday."-".$i]);
        if ($tm < 200){
          $cmd = trim($post["scm-".$rday."-".$i]);
          $schstr .= $id.$idx." ".day2str($day)." xx:xx ".$cmd." ".$day." ".($tm*4)."\n";
          $idx++;
        }
      }
    }
  }
  $sch= parseSchedule(explode("\n",$schstr));
  return $sch;
}

function readSchedule($hk,$sub){
  $hk .= " getschedule";

#  var_dump(($hk." ".$sub));

  return parseSchedule(doEmsCommand($hk." ".$sub));
}

function schedule2emsdata($sch){
  $result = array();
  
  $sinf = array();
  foreach($sch as $k => $v){
    if (!isset($sinf[$day])) $sinf[$day] = array();
    $dhm = time2dayhm($k);
    $day = $dhm[0];
    $t15 = $dhm[2];
    $cmd = $v;

    $sinf[$day][] = array("t15"=>$t15,"cmd"=>$cmd);
  }


  for($day=0;$day<=6;$day++){

    for ($i=0;$i<6;$i++){
    
        if (isset($sinf[$day][$i])){

          $timecode = $sinf[$day][$i]["t15"];
          $cmd = substr($sinf[$day][$i]["cmd"],1);
        
        } else {
          $timecode = "unset";
          $cmd = "";
        }
        
        $index = $i + 1;
        
        if (!isset($result[$day])) $result[$day] = array();
        $result[$day][$index] = array("timecode" => $timecode, "cmd" => $cmd);
    }
  }
  return $result;


}


function programSchedule($sch, $hk, $sub) {
    $newemsdata = schedule2emsdata($sch);
    $maxRetries = 5;
    $delayBetweenCommands = 300000;  // 300ms zwischen Commands (µs)
    $delayOnError = 500000;          // 500ms bei Fehler
    $delayOnRetry = 2000000;         // 2s vor jedem Retry-Durchgang

    for ($retry = 0; $retry < $maxRetries; $retry++) {

        if ($retry > 0) {
            usleep($delayOnRetry);
        }

        $oldemsdata = schedule2emsdata(readSchedule($hk, $sub));
        $errorOccurred = false;
        $written = false;

        for ($day = 0; $day <= 6; $day++) {
            if ($errorOccurred) break;

            for ($index = 1; $index <= 6; $index++) {
                print('<script type="text/javascript">progress(' . (($index - 1 + $day * 6) * 2) . ');</script>' . "\n");

                $tc1  = $oldemsdata[$day][$index]["timecode"];
                $tc2  = $newemsdata[$day][$index]["timecode"];
                $cmd1 = $oldemsdata[$day][$index]["cmd"];
                $cmd2 = $newemsdata[$day][$index]["cmd"];

                if (($tc1 != $tc2) || ($cmd1 != $cmd2)) {
                    $written = true;
                    $eqry = "$hk programschedule $sub $day $index $tc2 $cmd2";
                    $eres = doEmsCommand($eqry);
                    $eres = trim(end($eres));

                    if ($eres != "OK") {
                        // Fehler – nicht abbrechen, Retry-Loop nutzen
                        error_log("programSchedule: Fehler bei $eqry (Antwort: '$eres'), Retry " . ($retry + 1) . "/$maxRetries");
                        usleep($delayOnError);
                        $errorOccurred = true;
                        break; // innere Schleife abbrechen, Retry starten
                    }

                    usleep($delayBetweenCommands); // UI800 Luft lassen
                }

                flush_buffers();
            }
        }

        if (!$errorOccurred && !$written) {
            return true; // alles passt, fertig
        }

        if (!$errorOccurred && $written) {
            // alles geschrieben ohne Fehler – zur Sicherheit einmal verifizieren
            $verifydata = schedule2emsdata(readSchedule($hk, $sub));
            if ($verifydata === $newemsdata) {
                return true; // Verifikation OK
            }
            error_log("programSchedule: Verifikation fehlgeschlagen, Retry " . ($retry + 1) . "/$maxRetries");
        }
    }

    // Alle Retries aufgebraucht
    print("<table bgcolor='#FF0000' cellpadding=10><tr><td><h3>Schreiben fehlgeschlagen nach $maxRetries Versuchen!</h3></td></tr></table>");
    return false;
}

function getActSchedule($src){
  $srch = array("ww" => "ww customschedule",
                "zir" => "zirkpump customschedule");

  $trans = array("off" => "hk",
                 "on" => "custom");

  if (($src == "ww") || ($src == "zir")){
    doEmsDataCommand("ww requestdata");
    $res = getEmsLiveData();
    return $trans[$res[$srch[$src]]];
  }

  $res = doEmsCommand($src." getactiveschedule");
  array_pop($res);
  return (trim(array_pop($res)));
}

function getEmsStatusCodes(){
  global $mysql_host, $mysql_user, $mysql_pass, $log_max_lines;
  $link=new mysqli($mysql_host, $mysql_user, $mysql_pass, "ems_data")
     or die("Keine Verbindung zur Datenbank möglich: " . mysqli_error($link));
     
#  $resultq = $link->prepare("SELECT a.value as sc, b.value as fc, a.starttime as time ".
#                      "FROM state_data a, state_data b ".
#                      "WHERE a.sensor = 200 and b.sensor = 201 ".
#                      "       and a.starttime = b.starttime ".
#                      "ORDER BY time DESC LIMIT 0,".$log_max_lines )
#        or die("Abfrage fehlgeschlagen! ". mysqli_error($link));
#

  $resultq = $link->prepare("SELECT '--' as sc, value as fc, starttime as time ".
                      "FROM state_data  ".
                      "WHERE (sensor = 201) OR (sensor = 203) ".
                      "ORDER BY time DESC LIMIT 0,".$log_max_lines )
        or die("Abfrage fehlgeschlagen! ". mysqli_error($link));
  $resultq->execute();         
  $result = $resultq->get_result();       
  $res = array();
  while($row = $result->fetch_assoc() ){
    $res[]=str_replace(" ","|",trim($row['time']))."|".$row['sc']."|".$row['fc'];
  }                                                   
  return $res;
}                                                   

function ldat_parse($s){

  if (substr($s,-4) == " min"){
    $h = $s;
    $s = str_replace(" min","",$s);
    $s2 = ($s % (60));
    $s = $s - $s2;
    $s = $s / (60);
    $s = $s."h ".$s2."min";
    }
  return $s;

}

function getEmsLiveData($withtimes=false){
  $in = doEmsDataCommand();
  $inf = array();
  foreach ($in as $z){
    $p = strpos($z,"=");
    $zi = array(trim(substr($z,0,$p)),substr($z,$p+2));
    $k = trim($zi[0]);
    $v = trim($zi[1]);
    if (!$withtimes){
      $va = explode("|",$v);
      $v = trim($va[0]);
    }

    if ($k && (strpos(" ".$z,"error ")==FALSE)) $inf[$k] = $v;
  }

  ksort($inf);
  return $inf;
}

function diff2col($diff,$limit,$low,$med,$high){
  if ($diff < $limit/2){
    return d2c($diff, $limit/2, $low, $med);
  } else {
    return d2c($diff-$limit/2, $limit/2, $med, $high);  
  }
}

function d2c($diff,$limit,$low,$high){
  $rs = "#";
  for ($i=0 ; $i<3; $i++){
    if ($diff > $limit ) { 
      $res = $high[$i];
    } else {
      $cdiff = ($high[$i]-$low[$i]);
      $res = $low[$i] + ($diff/$limit) * $cdiff;
    }
    $rs.=substr("00".dechex($res),-2);
  }
  return $rs;
}
