<?php

$router = Lxx\Application::app()->getRouter();

$router->get("/", "Index@Index");
$router->get("/one/{name}", "Index@One");
$router->get("/two", "Index@Two");