<?php

$router = Lxx\Application::app()->router();

$router->get("/user/b/c/*action", "Index@One");

$router->get("/user/member/{id}", "Index@Two");

$router->get("/user/b/{a}/{id}", function($app){
    return "Hello 33!\n";
});

$router->post("/user/b/{a}", function(){
    echo "Hello 44!\n";
});

$router->get("/a-{abc}", "Index@Index");

// $router->get("/a-{a}", function(){
//     echo "Hello 55!\n";
// });

$router->get("/user/b/a", function(){
    echo "Hello 66!\n";
});


$router->get("/user/c/c/{name}/a", function(){
    echo "Hello 77!\n";
});

$router->get("/user/{name}/abc", function(){
    echo "Hello 88!\n";
});