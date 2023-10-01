<?php   
    class Conexion extends PDO {
        private $hostBD = 'sql9.freemysqlhosting.net';
        private $nombreBD = 'sql9580791';
        private $usuarioBD = 'sql9580791';
        private $passwordBD = 'XbtmQLMmpe';

        public function __construct(){
            try {
                parent::__construct('mysql:host=' . $this->hostBD .'; dbname=' . $this->nombreBD . ';charset=utf8', $this->usuarioBD, $this->passwordBD,
                array(PDO::ATTR_ERRMODE=>PDO::ERRMODE_EXCEPTION));
            } catch (PDOException $e) {
                echo 'Error: '. $e->getMessage();
            }
        }
    }
?>