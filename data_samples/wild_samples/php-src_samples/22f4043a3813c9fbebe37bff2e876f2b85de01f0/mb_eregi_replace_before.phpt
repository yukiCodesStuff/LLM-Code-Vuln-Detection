    foreach($replacement as $i=>$u) {
        $st = mb_eregi_replace($i,$u,$st);
    }
    return $st;
} 

echo do_translit("Фуцк");
?>
--EXPECT--
Fuck
--CREDITS--
Testfest Wuerzburg 2009-06-20