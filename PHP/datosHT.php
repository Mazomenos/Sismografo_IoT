
<?php
    include 'conexion.php';
    $pdo = new Conexion();
    if($_SERVER['REQUEST_METHOD'] == 'GET') {
       if(isset($_GET['t'])){
        $sql = "INSERT INTO IoTFinalHT(Modulo,temperatura,humedad) VALUES 
(:m, :t, :h)";
        $stmt = $pdo->prepare($sql);
        $stmt->bindValue(':m', $_GET['m']);
        $stmt->bindValue(':t', $_GET['t']);
        $stmt->bindValue(':h', $_GET['h']);
        $stmt->execute();
        header("HTTP/1.1 200 OK");
        exit;
       }
       else{
        $sql = $pdo->prepare("SELECT * FROM IoTFinalHT");
        $sql->execute();
        $sql->setFetchMode(PDO::FETCH_ASSOC);
        header("HTTP/1.1 200 OK");
        echo json_encode($sql->fetchAll());
        exit;
       }
    }
    if($_SERVER['REQUEST_METHOD'] == 'POST') {
        $sql = "INSERT INTO IoTFinalHT(Modulo,temperatura,humedad) VALUES 
        (:m, :t, :h)";
        $stmt = $pdo->prepare($sql);
        $stmt->bindValue(':m', $_POST['m']);
        $stmt->bindValue(':t', $_POST['t']);
        $stmt->bindValue(':h', $_POST['h']);
        $stmt->execute();
        $idPost = $pdo->lastInsertId();
        if($idPost) {
            header("HTTP/1.1 200 OK");
            echo json_encode($idPost);
            exit;
        }
    }
?>