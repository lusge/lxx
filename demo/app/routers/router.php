<?php

$router = Lxx\Application::app()->getRouter();

$router->get("/", "Index@Index");