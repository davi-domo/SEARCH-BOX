// constante 
const path_esp = "./";
// path a decommenter pour les test en local d'interogation de la base de donnée
//const path_esp = "http://search-box.local/";
const result_search = document.querySelector('#result_search');
const box_search = document.querySelector('#box_search');
const modif_nb_stock = document.querySelector('#modif_nb_stock');
const add_stock = document.querySelector('#add_stock');
const disabled = document.querySelector('#disabled');

// fonction encode_car
function encode_car(car) {
    car.replaceAll("'", "<apos>");
    car.replaceAll("<br>", "\n");
    return car;
}

function decode_car(car) {
    car.replaceAll("<apos>", "'");
    car.replaceAll("\n", "<br>");

}


// fontion add_search
//Ajout des tableau a chaque reponse du moteur de recherche
function add_search(id, nom, categorie, detail, stock, stock_alert, tiroir, image) {
    let span_stock = "<span style='float:right;'>Stock : " + stock + "</span>";
    if (stock <= stock_alert) { span_stock = "<span class='alert' style='float:right;'>Stock : " + stock + "</span>"; }

    let table = "<div id='" + id + "' class='result_search_id' onclick='detail(this.id)'>";
    table += "<table class='table_result_search' >";
    table += "<tbody>";
    table += "<tr>";
    table += "<td valign='top' rowspan ='3' style='width: 6%;'><img src='" + image.replaceAll(" ", "+") + "' style='width:3vmax;'></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td colspan='2' valign='top' style='height:1vmax;'>" + nom + "<span style='position:fixed;left:50%;'>Catégorie : " + categorie + "</span>" + span_stock + "</td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td valign='top' class='detail'>Détail : " + detail.replaceAll("<apos>", "'") + "</td>";
    table += "<td valign='top' style='text-align: right;width:10%;'>Tiroir : " + tiroir + "</td>";
    table += "</tr>";
    table += "</tbody>";
    table += "</table >";
    table += "</div> ";
    return table;
}

// fonction load_search
// fonction asynchrone d'interogation de la base de donnée avec reponse Json
async function load_search(arg) {
    const response = await fetch(path_esp + 'search?req=' + arg);
    const data = await response.json();
    result_search.innerHTML = "";
    let box_search_lower = box_search.value.toLowerCase();
    // on interoge chaque neaud du Json
    for (i = 0; i < data.length; i++) {
        if (data[i].id >= 0) {
            let return_nom = data[i].nom.toLowerCase().replace(box_search_lower, "<strong style='color:rgb(63, 82, 251,0.9);'>$&</strong>");
            let return_detail = data[i].detail.toLowerCase().replace(box_search_lower, "<strong style='color:rgba(63, 82, 251, 0.9);'>$&</strong>");
            let return_categorie = data[i].categorie.toLowerCase().replace(box_search_lower, "<strong style='color:rgb(63, 82, 251,0.9);'>$&</strong>");
            result_search.innerHTML += add_search(data[i].id, return_nom, return_categorie, return_detail, data[i].stock, data[i].stock_alert, data[i].tiroir, data[i].image);
        }
    }
    result_search.style.display = 'block';
    timer = false;
}

// ecoute de la boite search avec timer pour évité les surchage de l'esp32
// on ecoute le champ search
let timer = false;
let repos;
function timeout_req() {
    if (timer == false) {
        // création d'un timer afin de laisser le temps de frappe avant interogation
        repos = setTimeout(function () { load_search(box_search.value) }, 600);
        timer = true;
    }
}
function req_search() {
    //si plus de 2 caracteres
    if (box_search.value.length > 2) {
        if (timer == true) {
            // si timer deja actif on efface et relance
            clearTimeout(repos);
            timer = false;
            timeout_req();
            console.log(timer);
        }
        else {
            timeout_req();
        }
    }
    else {
        result_search.style.display = 'none';

    }
}



// fonction add_list 
// ajoute une ligne a chaque reponse
function add_list(id, nom, categorie, detail, stock, stock_alert, tiroir) {
    let td_stock = "<td>" + stock + "</td>";
    if (stock <= stock_alert) { td_stock = "<td class='alert'>" + stock + "</td>"; }
    let table = "<tr id='" + id + "' onclick='detail(this.id)'>";
    table += "<td>" + categorie + "</td>";
    table += "<td>" + nom + "</td>";
    table += "<td title='" + detail.replaceAll("<apos>", "&apos;") + "' class='detail'>" + detail.replaceAll("<apos>", "&apos;") + "</td>";
    table += td_stock;
    table += "<td>" + tiroir + "</td>";
    table += "</tr>";
    return table;
}
// fonction load_list
// fonction asyncrone d'interogation de la base de donnée
async function load_list() {
    let response = await fetch(path_esp + 'list');
    const data = await response.json();
    box_search.value = "";
    result_search.innerHTML = "";
    let table_list = "<table class='list'>";
    table_list += "<thead>";
    table_list += "<tr>";
    table_list += "<th style='width:20%'>Catégorie</th>";
    table_list += "<th style='width:20%'>Nom</th>";
    table_list += "<th >Détail</th>";
    table_list += "<th style='width:10%'>Stock</th>";
    table_list += "<th style='width:10%'>Tiroir</th>";
    table_list += "</tr>";
    table_list += "</thead>";
    table_list += "<tbody>";
    for (i = 0; i < data.length; i++) {
        if (data[i].id >= 0) {

            table_list += add_list(data[i].id, data[i].nom, data[i].categorie, data[i].detail, data[i].stock, data[i].stock_alert, data[i].tiroir);
            //console.log(return_nom);
        }
    }
    table_list += "</tbody>";
    table_list += "</table>";
    result_search.innerHTML += table_list;
    result_search.style.display = 'block';
}


// fonction de gestion de la fiche de détail
// fonction de gestion de liste
// fonction load_detail arg => ID de la base
// fonction asyncrone d'interogation de la base de donnée
async function load_detail(arg) {
    let response = await fetch(path_esp + 'detail?id=' + arg);
    const data = await response.json();
    box_search.value = "";
    result_search.innerHTML = "";
    let detail = data[0].detail.replaceAll("<apos>", "&apos;");
    let span_stock = "<span class='stock_ok' style='float:right;'>Stock : " + data[0].stock + "</span>";
    if (data[0].stock <= data[0].stock_alert) { span_stock = "<span class ='alert' style='float:right;'>Stock : " + data[0].stock + "</span>"; }
    let table = "<table class='detail'>";
    table += "<thead>";
    table += "<tr>";
    table += "<th>" + data[0].nom + "</th>";
    table += "<th align='right' colspan='2'><button  onclick='locate_box(" + data[0].id + ",\"" + data[0].nom + "\"," + data[0].stock + "," + data[0].stock_alert + "," + data[0].tiroir + ")'><img src='./img/ico_loc.png' id='loc_box'><span>Localiser</span></button></td>";
    table += "</tr>";
    table += "</thead>";
    table += "<tbody>";
    table += "<tr>";
    table += "<td rowspan='3' align='center' width='40%'><img class='img_detail' src='" + data[0].image.replaceAll(" ", "+") + "' style='width:80%;'></td>";
    table += "<td valign='top' height='1vmax' colspan='2'>Categorie : " + data[0].categorie + "" + span_stock + "</span></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td valign='top' colspan='2'>Description : <br> " + detail + "</td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td colspan='2' valign='bottom' align='right'  height='1vmax'>Tiroir : " + data[0].tiroir + "</td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td ><button onclick='del_id(" + data[0].id + ",\"" + data[0].nom + "\");' ><img src='./img/ico_del.png' id='img_del'><span>Supprimer</span></button></td>";
    table += "<td colspan='2' align='right' onclick='add_upd_elm(\"update\",\"" + data[0].id + "\",\"" + data[0].categorie + "\",\"" + data[0].nom + "\",\"" + data[0].detail.replaceAll("'", "<apos>") + "\"," + data[0].stock + "," + data[0].stock_alert + "," + data[0].tiroir + ",\"" + data[0].image.replaceAll(" ", "+") + "\");'><button><img src='./img/ico_mod.png'><span>Modifier</span></button></td>";
    table += "</tbody>";
    table += "</table>";
    result_search.innerHTML = table;
    result_search.style.display = 'block';
}

// fonction detail 
// appel de la fonction load_detail pour les onclick des tr du tableau
function detail(id) {
    load_detail(id);
}

// fonction de localisation
// fonction locate_box(arg)
// arg = numero de tiroir
async function locate_box(id, nom, stock, stock_alert, tiroir) {
    document.querySelector('#loc_box').src = "./img/loc_anim.gif";
    let response = await fetch(path_esp + 'locate_box?tiroir=' + tiroir);
    const data = await response.json();
    // si on recoit la confirmation
    if (data[0].result != -1) {
        // on affiche la box de mise a jour de stock
        let table = "<table class='maj_nb'>";
        table += "<thead>";
        table += "<tr>";
        table += "<th>" + nom + "</th>";
        table += "<th align='right'><button  onclick='close_windows(\"modif_nb_stock\");'><img src='./img/ico_close.png'><span>Fermer</span></button></td>";
        table += "</tr>";
        table += "</thead>";
        table += "<tbody>";
        table += "<tr>";
        table += "<td align='right' width='50%' height='1vmax'><label for='nb_stock'>Stock : </label></td>";
        table += "<td align='left'><input type='number' id='loc_nb_stock' name='loc_nb_stock' min='0' value='" + stock + "'></td>";
        table += "</tr>";
        table += "<tr>";
        table += "<td align='right' width='50%' height='1vmax'><label for='stock_alert'>Stock minimun : </label></td>";
        table += "<td align='left'><input type='number' id='loc_stock_alert' name='loc_stock_alert' min='0' value='" + stock_alert + "'></td>";
        table += "</tr>";
        table += "<tr>";
        table += "<td align='right' width='50%' height='1vmax'><label for='tiroir'>Tiroir : </label></td>";
        table += "<td align='left'><input type='number' id='loc_tiroir' name='loc_tiroir' min='0' value='" + tiroir + "'></td>";
        table += "</tr>";
        table += "<tr>";
        table += "<td ><button onclick='del_id(" + id + ",\"" + nom + "\");'><img src='./img/ico_del.png' id='img_del_loc'><span>Supprimer</span></button></td>";
        table += "<td colspan='2' align='right'><button id='valid_stock'><img src='./img/ico_mod.png' id='img_valid'><span>Valider</span></button></td>";
        table += "</tbody>";
        table += "</table>";
        modif_nb_stock.innerHTML = table;
        disabled.style.display = 'block';
        modif_nb_stock.style.display = 'block';

        let btn_send = document.querySelector('#valid_stock');
        btn_send.addEventListener('click', function (e) {
            let loc_stock_num = document.querySelector('#loc_nb_stock').value;
            let loc_stock_alert = document.querySelector('#loc_stock_alert').value;
            let loc_tiroir = document.querySelector('#loc_tiroir').value;
            let req_loc_udp = path_esp + 'update_loc?stock=' + loc_stock_num + '&stock_alert=' + loc_stock_alert + '&tiroir=' + loc_tiroir + '&id=' + id;
            fetch(req_loc_udp);
            modif_nb_stock.style = 'opacity:0;transition-duration: 6s';
            document.querySelector('#loc_box').src = "./img/ico_loc.png";
            setTimeout(() => {
                modif_nb_stock.style = '';
                modif_nb_stock.style.display = 'none';
                disabled.style.display = 'none';
                load_list();
            },
                6000);

        });
    }


}

function close_windows(fenetre) {

    let close = document.querySelector('#' + fenetre);
    //on envoie une requete si fenetre modif_nb_stock pour éteindre la led tiroir
    if (fenetre == "modif_nb_stock") {
        let close_led = path_esp + 'close_led';
        fetch(close_led);
        close.style = 'opacity:0;transition-duration: 2s';
        document.querySelector('#loc_box').src = "./img/ico_loc.png";
        setTimeout(() => {
            disabled.style.display = 'none';

            close.style = '';
            close.style.display = 'none';
        },
            2000);
    }
    else {

        disabled.style.display = 'none';
        close.style.display = 'none';
    }

}

function del_id(id, nom) {
    if (confirm("Voulez vous vraiment supprimmer :\n     - " + nom + "\nDéfinitivement !") == true) {

        fetch(path_esp + 'delete?id=' + id);
        modif_nb_stock.style = 'opacity:0;transition-duration: 6s';
        setTimeout(() => {
            modif_nb_stock.style = '';
            modif_nb_stock.style.display = 'none';
            result_search.style.display = 'none';
            disabled.style.display = 'none';
            load_list();
        },
            6000);
    }
}


/*** gestion ajout nouvelle element ***/
function add_upd_elm(type, elm_id = 0, elm_categorie = "", elm_nom = "", elm_detail = "", elm_stock = "", elm_stock_alert = 0, elm_tiroir = 1, elm_img = "") {
    let titre = "Ajouter un objet";
    let txt_send = "Ajouter";
    if (type == "update") {
        titre = "Modification de : " + elm_nom;
        txt_send = "Modifier";
        elm_detail = elm_detail.replaceAll("<br>", "\n");

        elm_detail = elm_detail.replaceAll("<apos>", "'");
    }

    let table = "<table class='add_stock'>";
    table += "<thead>";
    table += "<tr>";
    table += "<th>" + titre + "</th>";
    table += "<th align='right'><button  onclick='close_windows(\"add_stock\");'><img src='./img/ico_close.png'><span>Fermer</span></button></td>";
    table += "</tr>";
    table += "</thead>";
    table += "<tbody>";
    table += "<tr>";
    table += "<td align='right' valign='top' width='30%' height='1vmax'><label for='add_file' class='label_file'><img src='./img/ico_add.png'><span>Selectionnez une image</span></label><input type='file' id='add_file' name='add_file' accept='image/*'  class ='input-file'></td>";
    table += "<td align='center'> <img id='preview' src ='" + elm_img + "'height='64px'></img></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td align='right' height='1vmax'><label for='add_categorie'>Categorie : </label></td>";
    table += "<td align='left'><input type='text' id='add_categorie' name='add_categorie' value='" + elm_categorie + "'></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td align='right' height='1vmax'><label for='add_nom'>Nom : </label></td>";
    table += "<td align='left'><input type='text' id='add_nom' name='add_nom' value='" + elm_nom + "'></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td  colspan='2' align='left' height='1vmax' style='padding-bottom:0px;><label for='add_detail'>Description : </label></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td colspan='2' align='center' style='padding-top:0px;width:96%;'><textarea id='add_detail' name='add_detail' row='4' style='width:96%;'>" + elm_detail + "</textarea></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td align='right' height='1vmax'><label for='add_stock_num'>Stock : </label></td>";
    table += "<td align='left'><input type='number' id='add_stock_num' name='add_stock_num' min='0'  value='" + elm_stock + "'></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td align='right' height='1vmax'><label for='add_stock_alert'>Stock mini : </label></td>";
    table += "<td align='left'><input type='number' id='add_stock_alert' name='add_stock_alert' min='0'  value='" + elm_stock_alert + "'></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td align='right' height='1vmax'><label for='add_tiroir'>Tiroir : </label></td>";
    table += "<td align='left'><input type='number' id='add_tiroir' name='add_tiroir' min='1'  value='" + elm_tiroir + "'></td>";
    table += "</tr>";
    table += "<tr>";
    table += "<td ></td>";
    table += "<td colspan='2' align='right'><button id='valid_send'><img src='./img/ico_mod.png' id='img_valid'><span>" + txt_send + "</span></button></td>";
    table += "</tbody>";
    table += "</table>";
    add_stock.innerHTML = table;
    disabled.style.display = 'block';
    add_stock.style.display = 'block';


    let imgInput = document.querySelector('#add_file');
    let add_img = "";
    imgInput.addEventListener('change', function (e) {
        if (e.target.files) {
            let imageFile = e.target.files[0];

            let reader = new FileReader();
            reader.onload = function (e) {
                let img = document.createElement("img");
                img.onload = function (event) {
                    let MAX_WIDTH = 256;
                    let MAX_HEIGHT = 256;

                    let width = img.width;
                    let height = img.height;
                    // Change the resizing logic
                    if (width > height) {
                        if (width > MAX_WIDTH) {
                            height = height * (MAX_WIDTH / width);
                            width = MAX_WIDTH;
                        }
                    } else {
                        if (height > MAX_HEIGHT) {
                            width = width * (MAX_HEIGHT / height);
                            height = MAX_HEIGHT;
                        }
                    }

                    let canvas = document.createElement("canvas");
                    canvas.width = width;
                    canvas.height = height;
                    let ctx = canvas.getContext("2d");
                    ctx.drawImage(img, 0, 0, width, height);

                    data_url = canvas.toDataURL('image/webp');
                    document.getElementById("preview").src = data_url;
                }
                img.src = e.target.result;
            }
            reader.readAsDataURL(imageFile);
        }
    });
    let btn_send = document.querySelector('#valid_send');
    btn_send.addEventListener('click', function (e) {

        let add_categorie = document.querySelector('#add_categorie').value;
        let add_nom = document.querySelector('#add_nom').value;
        let add_detail = document.querySelector('#add_detail').value.replaceAll("'", "<apos>");
        let add_stock_num = document.querySelector('#add_stock_num').value;
        let add_stock_alert = document.querySelector('#add_stock_alert').value;
        let add_tiroir = document.querySelector('#add_tiroir').value;
        add_detail = add_detail.replaceAll("\n", '<br>');
        add_img = document.getElementById("preview").src;
        let req_add_udp = path_esp + type + '?categorie=' + add_categorie + '&nom=' + add_nom + '&detail=' + add_detail + '&stock=' + add_stock_num + '&stock_alert=' + add_stock_alert + '&tiroir=' + add_tiroir + '&img=' + add_img;
        if (type == 'update') { req_add_udp += '&id=' + elm_id }
        fetch(req_add_udp);

        add_stock.style = 'opacity:0;transition-duration: 6s';
        setTimeout(() => {

            add_stock.style = '';
            add_stock.style.display = 'none';
            disabled.style.display = 'none';
            load_list();
        },
            6000);

    });
}
