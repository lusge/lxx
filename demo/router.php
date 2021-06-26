<?php

class Router {
    public $raxTree;
    private $paths;
    private $opts;
    private $index = -1;
    private $matchData = [];

    public function __construct() {
        $this->raxTree = new Lxx\RadixTree();
    }

    public function GET(string $path, array $opts ) :void {
        $routeOpts = [];
        $pos = strpos($path, ':');
        if ($pos) {
            $newPath = substr($path, 0, $pos - 1);
            $routeOpts["op"] = "<=";
            $routeOpts["path"] = $newPath;
            $routeOpts["param"] = true;
        } else {
            $pos = strpos($path, "*");
            if ($pos) {
                if ($pos != strlen($path)) {
                    $routeOpts["param"] = true;
                }
                $newPath = substr($path, 0, $pos - 1);
                $routeOpts["op"] = "<=";
                $routeOpts["path"] = $newPath;
            } else {
                $routeOpts["op"] = "=";
            }
            $routeOpts["path"] = $path;
        }

        $this->insertRoute($routeOpts);
    }

    private function insertRoute(array $opts) {
        $path = $opts["path"];
        if ($opts["op"] == "=") {
            $this->paths[$path] = $opts;
            return true;
        }

        $idx = $this->raxTree->find($path);
        if ($idx != false) {
            return true;
        }

        $this->index++;
        $this->matchData[$this->index] = $opts;
        $this->raxTree->insert($path, $this->index);
    }

    public function match($path) {
        $this->matchRoute($path);
    }

    private function matchRoute($path) {
        if (isset($this->paths[$path])) {
            var_dump($this->paths[$path]);
            return;
        }
        
        $this->raxTree->seek("<=", $path);
        while(true) {
            $idx = $this->raxTree->pre();
            if ($idx === false) {
                echo "match break\n";
                break;
            }

            $routes = $this->matchData[$idx];
            if (!empty($routes)) {
                var_dump($routes);
                return;
            }
        }
    }
}

$route = new Router();
$route->GET("/a/c/:name/:user", []);
$route->GET("/a/b/:name/:user", []);
$route->GET("/a/b/:c/:user", []);
$route->GET("/a/:name/:user", []);
$route->GET("/b/c/:name/:user", []);

$route->match("/a/b/a/sdfsdf");