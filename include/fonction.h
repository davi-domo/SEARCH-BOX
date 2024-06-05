
/***   Fonction de connection a la database   ***/
int openDb(const char *filename, sqlite3 **db)
{
    int rc = sqlite3_open(filename, db);
    Serial.printf("Ouverture de la database => ");
    if (rc)
    {
        Serial.printf("[ERROR] => %s\n", sqlite3_errmsg(*db));
        return rc;
    }
    else
    {
        Serial.printf("[SUCESS]\n");
    }
    return rc;
}
/*************************************************************************/
char *zErrMsg = 0;
const char *data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName)
{
    int i;
    Serial.printf("%s: ", (const char *)data);
    for (i = 0; i < argc; i++)
    {
        Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    Serial.printf("\n");
    return 0;
}
int db_exec(sqlite3 *db, const char *sql)
{
    Serial.println(sql);
    long start = micros();
    int rc = sqlite3_exec(db, sql, callback, (void *)data, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        Serial.printf("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        Serial.printf("Operation done successfully\n");
    }
    Serial.print(F("Time taken:"));
    Serial.println(micros() - start);
    return rc;
}

boolean extract_id()
{
    boolean return_rep = true;
    openDb(database, &db1);
    String sql = "SELECT id FROM liste_mat ORDER BY id DESC LIMIT 1";
    rc = sqlite3_prepare_v2(db1, sql.c_str(), -1, &res, &tail);

    if (rc != SQLITE_OK)
    {
        Serial.print(F("Failed to fetch data: "));
        Serial.print(sqlite3_errmsg(db1));
        return_rep = false;
    }

    while (sqlite3_step(res) == SQLITE_ROW)
    {
        id_table = sqlite3_column_int(res, 0);
    }

    sqlite3_finalize(res);
    sqlite3_close(db1);
    Serial.print(F("Id courante : "));
    Serial.println(id_table);

    return return_rep;
}

/***   Fonction de mise en forme de la réponse Json search   ***/
String search(String arg)
{
    String reponse_json;

    openDb(database, &db1);
    String sql = "SELECT * FROM liste_mat WHERE nom LIKE '%";
    sql += arg + "%' OR detail LIKE '%";
    sql += arg + "%'";

    rc = sqlite3_prepare_v2(db1, sql.c_str(), -1, &res, &tail);

    Serial.println(sql);
    reponse_json = "[";
    while (sqlite3_step(res) == SQLITE_ROW)
    {

        reponse_json += "{";
        reponse_json += "\"id\":";
        reponse_json += sqlite3_column_int(res, 0);
        reponse_json += ",";
        reponse_json += "\"categorie\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 1);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"nom\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 2);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"detail\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 3);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"stock\":";
        reponse_json += sqlite3_column_int(res, 4);
        reponse_json += ",";
        reponse_json += "\"stock_alert\":";
        reponse_json += sqlite3_column_int(res, 5);
        reponse_json += ",";
        reponse_json += "\"tiroir\":";
        reponse_json += sqlite3_column_int(res, 6);
        reponse_json += ",";
        reponse_json += "\"image\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 7);
        reponse_json += "\"";
        reponse_json += "},";
    }
    if (reponse_json.length() >= 2)
    {
        // on supprime la derniere virgule
        reponse_json = reponse_json.substring(0, reponse_json.length() - 1);
        // on ferme le tableau
        reponse_json += "]";
        sqlite3_finalize(res);
        sqlite3_close(db1);
    }
    else
    { // sinon on envoi id=-1
        reponse_json = "[{\"id\":-1}]";
    }

    return reponse_json;
}
/*************************************************************************/

/***   Fonction de mise en forme de la réponse Json list   ***/
String list()
{
    String reponse_json;

    openDb(database, &db1);
    String sql = "SELECT id, categorie, nom, detail, stock, stock_alert, tiroir FROM liste_mat ORDER BY categorie ASC";

    rc = sqlite3_prepare_v2(db1, sql.c_str(), -1, &res, &tail);

    if (rc != SQLITE_OK)
    {
        Serial.print(F("Failed to fetch data: "));
        Serial.print(sqlite3_errmsg(db1));
    }
    Serial.println(sql);
    reponse_json = "[";
    while (sqlite3_step(res) == SQLITE_ROW)
    {

        reponse_json += "{";
        reponse_json += "\"id\":";
        reponse_json += sqlite3_column_int(res, 0);
        reponse_json += ",";
        reponse_json += "\"categorie\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 1);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"nom\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 2);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"detail\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 3);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"stock\":";
        reponse_json += sqlite3_column_int(res, 4);
        reponse_json += ",";
        reponse_json += "\"stock_alert\":";
        reponse_json += sqlite3_column_int(res, 5);
        reponse_json += ",";
        reponse_json += "\"tiroir\":";
        reponse_json += sqlite3_column_int(res, 6);
        reponse_json += "},";
    }
    if (reponse_json.length() >= 2)
    {
        // on supprime la derniere virgule
        reponse_json = reponse_json.substring(0, reponse_json.length() - 1);
        // on ferme le tableau
        reponse_json += "]";
        sqlite3_finalize(res);
        sqlite3_close(db1);
    }
    else
    { // sinon on envoi id=-1
        reponse_json = "[{\"id\":-1}]";
    }

    return reponse_json;
}
/*************************************************************************/

/***   Fonction de mise en forme de la réponse Json search   ***/
String detail(String arg)
{
    String reponse_json;

    openDb(database, &db1);
    String sql = "SELECT * FROM liste_mat WHERE id = ";
    sql += arg;

    rc = sqlite3_prepare_v2(db1, sql.c_str(), -1, &res, &tail);

    Serial.println(sql);
    reponse_json = "[";
    while (sqlite3_step(res) == SQLITE_ROW)
    {

        reponse_json += "{";
        reponse_json += "\"id\":";
        reponse_json += sqlite3_column_int(res, 0);
        reponse_json += ",";
        reponse_json += "\"categorie\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 1);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"nom\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 2);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"detail\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 3);
        reponse_json += "\"";
        reponse_json += ",";
        reponse_json += "\"stock\":";
        reponse_json += sqlite3_column_int(res, 4);
        reponse_json += ",";
        reponse_json += "\"stock_alert\":";
        reponse_json += sqlite3_column_int(res, 5);
        reponse_json += ",";
        reponse_json += "\"tiroir\":";
        reponse_json += sqlite3_column_int(res, 6);
        reponse_json += ",";
        reponse_json += "\"image\": \"";
        reponse_json += (const char *)sqlite3_column_text(res, 7);
        reponse_json += "\"";
        reponse_json += "},";
    }
    if (reponse_json.length() >= 2)
    {
        // on supprime la derniere virgule
        reponse_json = reponse_json.substring(0, reponse_json.length() - 1);
        // on ferme le tableau
        reponse_json += "]";
        sqlite3_finalize(res);
        sqlite3_close(db1);
    }
    else
    { // sinon on envoi id=-1
        reponse_json = "[{\"id\":-1}]";
    }

    return reponse_json;
}
/*************************************************************************/

/***   Fonction de mise en forme de la réponse Json locate_box   ***/
String locate_box(String arg)
{

    String reponse_json = "[{\"result\": 1 }]";
    req_loc_box = arg.toInt();
    return reponse_json;
}
/*************************************************************************/

/***   Fonction de mise en forme de la réponse Json locate_box   ***/
String delete_id(int arg)
{
    String reponse_json;

    openDb(database, &db1);
    String sql = "DELETE FROM liste_mat WHERE id = ";
    sql += String(arg);

    rc = db_exec(db1, sql.c_str());
    if (rc != SQLITE_OK)
    {
        Serial.print(F("Failed to fetch data: "));
        Serial.print(sqlite3_errmsg(db1));
        reponse_json = "[{\"result\": 0 }]";
    }
    else
    {
        Serial.println(sql);
        sqlite3_close(db1);

        reponse_json = "[{\"result\": 1 }]";
    }

    req_loc_box = -1; // on reinit pour éteindre la localisation
    return reponse_json;
}
/*************************************************************************/

/***   Fonction de mise en forme de la réponse Json locate_box   ***/
String insert_data(String categorie, String nom, String detail, int stock, int stock_alert, int tiroir, String img)
{
    String reponse_json;
    id_table++;
    openDb(database, &db1);
    String sql = "INSERT INTO liste_mat (id, categorie, nom, detail, stock, stock_alert, tiroir, img) VALUES (";
    sql += id_table;
    sql += ",'";
    sql += categorie;
    sql += "','";
    sql += nom;
    sql += "','";
    sql += detail;
    sql += "',";
    sql += stock;
    sql += ",";
    sql += stock_alert;
    sql += ",";
    sql += tiroir;
    sql += ",'";
    sql += img;
    sql += "')";

    rc = db_exec(db1, sql.c_str());
    if (rc != SQLITE_OK)
    {
        Serial.print(F("Failed to fetch data: "));
        Serial.print(sqlite3_errmsg(db1));
        reponse_json = "[{\"result\": 0 }]";
    }
    else
    {
        // Serial.println(sql);
        sqlite3_close(db1);

        reponse_json = "[{\"result\": 1 }]";
    }

    return reponse_json;
}
/*************************************************************************/

/***   Fonction de mise en forme de la réponse Json locate_box   ***/
String update_data(int id, String categorie, String nom, String detail, int stock, int stock_alert, int tiroir, String img)
{
    String reponse_json;
    openDb(database, &db1);
    String sql = "UPDATE liste_mat set ";
    if (categorie != "")
    {
        sql += " categorie = '";
        sql += categorie;
        sql += "', nom = '";
        sql += nom;
        sql += "', detail = '";
        sql += detail;
        sql += "', stock = ";
        sql += stock;
        sql += ",stock_alert = ";
        sql += stock_alert;
        sql += ", tiroir =";
        sql += tiroir;
        sql += ", img = '";
        sql += img;
        sql += "' ";
    }
    else
    {

        sql += " stock = ";
        sql += stock;
        sql += ",stock_alert = ";
        sql += stock_alert;
        sql += ", tiroir =";
        sql += tiroir;
    }

    sql += " WHERE id =";
    sql += id;

    rc = db_exec(db1, sql.c_str());
    if (rc != SQLITE_OK)
    {
        Serial.print(F("Failed to fetch data: "));
        Serial.print(sqlite3_errmsg(db1));
        reponse_json = "[{\"result\": 0 }]";
    }
    else
    {
        // Serial.println(sql);
        sqlite3_close(db1);

        reponse_json = "[{\"result\": 1 }]";
    }
    udp_categorie = "";
    udp_nom = "";
    udp_detail = "";
    udp_stock = 0;
    udp_stock_alert = 0;
    udp_tiroir = 0;
    udp_img = "";
    req_loc_box = -1;
    return reponse_json;
}
/*************************************************************************/

/*************************************************************************/
/***   Fonction de gestion des led   ***/
// animation fin de localisation
void anim_led()
{
    Serial.println(F("animation led"));
    led = 0;
    for (int i = 0; i < max_led / 2; i = led)
    {
        leds.fadeToBlackBy(15);
        leds[scenario_led[led]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        leds[scenario_led[led++]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        leds[scenario_led[led++]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        leds[scenario_led[led++]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        leds[scenario_led[led++]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        leds[scenario_led[led++]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        leds[scenario_led[led++]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        leds[scenario_led[led++]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        leds[scenario_led[led++]] = CHSV(hue, 255, 180);
        leds[scenario_led[max_led - led]] = CHSV(hue, 255, 180);
        hue += 25;
        FastLED.show();
        delay(20);
    }
    for (int fin = 0; fin <= 80; fin++)
    {
        leds.fadeToBlackBy(15);
        FastLED.show();
        delay(10);
    }

    Serial.println(F("fin animation led"));
}
// action sur odre
void action_tiroir()
{
    // si on recois un odre de localisation et que le tiroir est eteint
    if (req_loc_box >= 1 && etat_loc_box == 0)
    {

        Serial.println(F("allumage tiroir"));
        etat_loc_box = 1;
        FastLED.clear();
        FastLED.show();
        for (int j = 0; j <= 255; j += 25)
        {
            for (int i = 0; i < nb_led_tiroir; i++)
            {
                leds[tiroir[req_loc_box - 1][i]] = CHSV(160, 255, j);
            }
            FastLED.show();
            delay(10);
        }
    }
    // si tiroir allumé et autre de fin
    if (req_loc_box == -1 && etat_loc_box == 1)
    {
        Serial.println(F("extinction tiroir"));

        etat_loc_box = 0;
        anim_led();
    }
}
