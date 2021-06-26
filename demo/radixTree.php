<?php

$radix = new Lxx\RadixTree();
// for ($i = 0; $i < 100; $i++) {
//     $tree->insert("/aaa".$i, ["a", "b", "d"]);
// }
// for ($i = 0; $i < 100; $i++) {
//     $find = $tree->find("/aaa".$i);
//     var_dump($find);
// }

$arr = ["/a/b/c", "/a/c/d", "/d/c/f"];
foreach ($arr as $index => $val) {
    // echo $index . " index \n";
    $radix->insert($val, $index);
}

$radix->seek("<=", '/d/c/f');

while(true) {
    $data = $radix->pre();
    if ($data === false) {
        echo "NO \n";
        break;
    }

    var_dump($data);
}

$find = $radix->find("/a/b/c");
var_dump($find);
