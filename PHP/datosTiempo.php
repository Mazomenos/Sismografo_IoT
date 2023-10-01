
<?php
    include 'conexion.php';
    $pdo = new Conexion();
    if($_SERVER['REQUEST_METHOD'] == 'GET') {
       if(isset($_GET['t'])){
        $sql = "INSERT INTO IoTFinalTime(Modulo, duracionSismo) VALUES 
(:m,:ma,:ds)";
        $stmt = $pdo->prepare($sql);
        $stmt->bindValue(':m', $_GET['m']);
        $stmt->bindValue(':ds', $_GET['ds']);
        $stmt->execute();
        header("HTTP/1.1 200 OK");
        exit;
       }
       else{
        $sql = $pdo->prepare("SELECT * FROM IoTFinalTime");
        $sql->execute();
        $sql->setFetchMode(PDO::FETCH_ASSOC);
        header("HTTP/1.1 200 OK");
        echo json_encode($sql->fetchAll());
        exit;
       }
    }
    if($_SERVER['REQUEST_METHOD'] == 'POST') {
        $sql = "INSERT INTO IoTFinalTime(Modulo, duracionSismo) VALUES 
        (:m,:ma,:ds)";
        $stmt = $pdo->prepare($sql);
        $stmt->bindValue(':m', $_POST['m']);
        $stmt->bindValue(':ds', $_POST['ds']);
        $stmt->execute();
        $idPost = $pdo->lastInsertId();
        if($idPost) {
            header("HTTP/1.1 200 OK");
            echo json_encode($idPost);
            exit;
        }
    }
?>