    foreach($replacement as $i=>$u) {
        $st = mb_eregi_replace($i,$u,$st);
    }
    return $st;
} 

echo do_translit("Пеар");
?>
--EXPECT--
pear
--CREDITS--
Testfest Wuerzburg 2009-06-20 (modified by rui 2011-10-15)