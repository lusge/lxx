<?php

$router = new Lxx\Router();
$router->get("/user/b/c/*action", function(){
    echo "Hello 11!\n";
});

$router->get("/user/member/{id}", function(){
    echo "Hello 22!\n";
});

$router->get("/user/b/{a}/{id}", function(){
    echo "Hello 33!\n";
});

$router->post("/user/b/{a}", function(){
    echo "Hello 44!\n";
});

$router->post("/a-{a}", function(){
    echo "Hello 55!\n";
});

$router->get("/user/b/a", function(){
    echo "Hello 66!\n";
});


$router->get("/user/c/c/{name}/a", function(){
    echo "Hello 77!\n";
});

$router->get("/user/{name}/abc", function(){
    echo "Hello 88!\n";
});

// $res = $router->match("GET", "/user/b/a");
$res = $router->match();
if ($res) {
    echo "Oh YES !!!\n";
    // var_dump($res);
    // $res["func"]();
} else {
    echo "Oh Fuck !!!\n";
}


// var_dump($router);