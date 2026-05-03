#include "../globals.hpp"
#include "htmlreport.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>

static const char* COLORES[] = {
    "#E53935",  // 0 -> rojo
    "#43A047",  // 1 -> verde
    "#1976D2",  // 2 -> azul
    "#FDD835",  // 3 -> amarillo
    "#00ACC1",  // 4 -> cyan
    "#AB47BC"   // 5 -> fucsia
};
static const int TOTAL_COLORES = 6;

static std::string fechaActual()
{
    const char* dias[]  = {"domingo","lunes","martes","miercoles","jueves","viernes","sabado"};
    const char* meses[] = {"enero","febrero","marzo","abril","mayo","junio",
                           "julio","agosto","septiembre","octubre","noviembre","diciembre"};
    std::time_t t  = std::time(nullptr);
    std::tm*    tm = std::localtime(&t);
    std::ostringstream ss;
    ss << dias[tm->tm_wday] << ", "
       << tm->tm_mday << " de " << meses[tm->tm_mon] << " de " << (1900 + tm->tm_year)
       << " - "
       << (tm->tm_hour < 10 ? "0" : "") << tm->tm_hour << ":"
       << (tm->tm_min  < 10 ? "0" : "") << tm->tm_min  << ":"
       << (tm->tm_sec  < 10 ? "0" : "") << tm->tm_sec;
    return ss.str();
}

bool htmlreport(std::vector<double> percVec, std::vector<int> quanVec)
{
    if (percVec.empty() || quanVec.empty()) return false;

    // ── Datos globales del sistema ───────────────────────────────────────────
    // quanstudents: total real de estudiantes en la BD (ya calculado en updateData())
    // quanVec: ultimo valor es noVotaron (SELECT COUNT WHERE Voto='0')
    // sinVotarPtr: apunta al objeto ABS. en partidosVec

    int totalEstudiantes = quanstudents;              // total real de la BD
    int noVotaron        = quanVec.back();            // ultimo de quanVec = no votaron
    int votaron          = totalEstudiantes - noVotaron; // los demas votaron algo
    int abstenciones     = noVotaron;
    int pctPartic        = (totalEstudiantes > 0) ? (votaron * 100 / totalEstudiantes) : 0;

    // ── Construir lista de partidos reales (excluye ABS.) ────────────────────
    struct PartidoReal { std::string nombre; int votos; double perc; };
    std::vector<PartidoReal> partidos;

    for (int i = 0; i < (int)partidosVec.size(); i++)
    {
        if (i >= (int)quanVec.size()) break;     // no pasar del ultimo (noVotaron)
//        bool esAbs = (sinVotarPtr != nullptr && partidosVec[i] == sinVotarPtr);
//        if (esAbs) continue;                          // excluir ABS. de los graficos
        PartidoReal p;
        p.nombre = partidosVec[i]->name;
        p.votos  = quanVec[i];
        p.perc   = (i < (int)percVec.size()) ? percVec[i] : 0.0;
        partidos.push_back(p);
    }

    // ── Nombre del laboratorio ───────────────────────────────────────────────
    std::string labStr = "";
    if (labName != nullptr && *labName != nullptr) labStr = *labName;
    if (labStr.empty()) labStr = "Sin definir";

    // ── Ruta del archivo de salida ───────────────────────────────────────────
    std::string outPath = "informe.html";
    if (informeName != nullptr && *informeName != nullptr)
    {
        outPath = *informeName;
        auto dot = outPath.rfind('.');
        if (dot != std::string::npos) outPath = outPath.substr(0, dot);
        outPath += ".html";
    }

    // ── Array JS con partidos reales ─────────────────────────────────────────
    std::ostringstream jsArr;
    jsArr << "[\n";
    for (int i = 0; i < (int)partidos.size(); i++)
    {
        const char* color = COLORES[i < TOTAL_COLORES ? i : TOTAL_COLORES - 1];
        std::string percStr = std::to_string(partidos[i].perc);
        if (percStr.length() > 4) percStr = percStr.substr(0, percStr.length() - 4);
        jsArr << "  { nombre:'" << partidos[i].nombre << "'"
              << ", votos:"     << partidos[i].votos
              << ", perc:'"     << percStr << "'"
              << ", color:'"    << color   << "' }";
        if (i + 1 < (int)partidos.size()) jsArr << ",";
        jsArr << "\n";
    }
    jsArr << "]";

    // ── HTML completo ────────────────────────────────────────────────────────
    std::ostringstream h;

    h << "<!DOCTYPE html>\n"
      << "<html lang=\"es\">\n"
      << "<head>\n"
      << "<meta charset=\"UTF-8\">\n"
      << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
      << "<title>Reporte de Votaciones VOCA 2026</title>\n"
      << "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/4.4.1/chart.umd.js\"></script>\n"
      << "<style>\n"
      << "* { box-sizing: border-box; margin: 0; padding: 0; }\n"
      << "body { font-family: Arial, sans-serif; background: #f4f6f9; }\n"
      << ".header { background: #1565C0; color: white; padding: 14px 24px; display: flex; justify-content: space-between; align-items: center; }\n"
      << ".header-title { font-size: 15px; font-weight: bold; }\n"
      << ".header-sub { font-size: 11px; opacity: 0.85; margin-top: 3px; }\n"
      << ".btn-pdf { background: white; color: #1565C0; border: none; border-radius: 6px; padding: 7px 16px; font-size: 12px; font-weight: bold; cursor: pointer; }\n"
      << ".btn-pdf:hover { background: #e3f2fd; }\n"
      << ".body { padding: 14px; }\n"
      << ".cards-top { display: grid; grid-template-columns: 1fr 1fr 1fr 1fr; gap: 12px; margin-bottom: 14px; }\n"
      << ".card { border-radius: 8px; padding: 14px 18px; color: white; display: flex; align-items: center; gap: 14px; }\n"
      << ".card.blue   { background: #1976D2; }\n"
      << ".card.green  { background: #388E3C; }\n"
      << ".card.orange { background: #E65100; }\n"
      << ".card.purple { background: #6A1B9A; }\n"
      << ".card-icon { width: 38px; height: 38px; background: rgba(255,255,255,0.22); border-radius: 50%; display: flex; align-items: center; justify-content: center; font-size: 17px; flex-shrink: 0; }\n"
      << ".card-num { font-size: 26px; font-weight: bold; line-height: 1; }\n"
      << ".card-label { font-size: 11px; opacity: 0.9; margin-top: 2px; }\n"
      << ".charts-row { display: grid; grid-template-columns: 1fr 1fr; gap: 12px; margin-bottom: 12px; }\n"
      << ".chart-card { background: white; border-radius: 8px; padding: 14px; border: 1px solid #ddd; margin-bottom: 12px; }\n"
      << ".chart-title { font-size: 12px; color: #444; margin-bottom: 8px; font-weight: bold; }\n"
      << ".legend { display: flex; flex-wrap: wrap; gap: 8px; margin-bottom: 8px; }\n"
      << ".legend-item { display: flex; align-items: center; gap: 4px; font-size: 11px; color: #555; }\n"
      << ".legend-dot { width: 10px; height: 10px; border-radius: 50%; flex-shrink: 0; }\n"
      << "table { width: 100%; border-collapse: collapse; font-size: 12px; }\n"
      << "th { background: #1565C0; color: white; padding: 7px 10px; text-align: left; }\n"
      << "td { padding: 6px 10px; border-bottom: 1px solid #eee; }\n"
      << "tr:nth-child(even) td { background: #f9f9f9; }\n"
      << ".badge { display: inline-block; padding: 2px 8px; border-radius: 12px; font-size: 11px; font-weight: bold; color: white; }\n"
      << ".footer { background: #1565C0; color: rgba(255,255,255,0.7); text-align: center; padding: 10px; font-size: 11px; }\n"
      << "@media print {\n"
      << "  body { background: white; }\n"
      << "  .btn-pdf { display: none !important; }\n"
      << "  .header, .card, th { -webkit-print-color-adjust: exact; print-color-adjust: exact; }\n"
      << "  @page { size: A4 landscape; margin: 8mm; }\n"
      << "}\n"
      << "</style>\n"
      << "</head>\n"
      << "<body>\n"
      << "<div style=\"border-radius:8px;overflow:hidden;border:1px solid #ccc;\">\n"

      << "<div class=\"header\">\n"
      << "  <div>\n"
      << "    <div class=\"header-title\">Sistema de Votaciones VOCA 2026 - Reporte de Resultados</div>\n"
      << "    <div class=\"header-sub\">" << fechaActual() << " | Laboratorio: " << labStr << "</div>\n"
      << "  </div>\n"
      << "  <button class=\"btn-pdf\" onclick=\"window.print()\">Descargar PDF</button>\n"
      << "</div>\n"

      << "<div class=\"body\">\n"

      << "<div class=\"cards-top\">\n"
      << "  <div class=\"card blue\"><div class=\"card-icon\">&#x1F465;</div>"
      <<    "<div><div class=\"card-num\">" << totalEstudiantes << "</div>"
      <<    "<div class=\"card-label\">Total Estudiantes</div></div></div>\n"
      << "  <div class=\"card green\"><div class=\"card-icon\">&#x2713;</div>"
      <<    "<div><div class=\"card-num\">" << votaron << "</div>"
      <<    "<div class=\"card-label\">Votaron</div></div></div>\n"
      << "  <div class=\"card orange\"><div class=\"card-icon\">&#x26A0;</div>"
      <<    "<div><div class=\"card-num\">" << abstenciones << "</div>"
      <<    "<div class=\"card-label\">Abstenciones</div></div></div>\n"
      << "  <div class=\"card purple\"><div class=\"card-icon\">&#x25B6;</div>"
      <<    "<div><div class=\"card-num\">" << pctPartic << "%</div>"
      <<    "<div class=\"card-label\">Participacion</div></div></div>\n"
      << "</div>\n"

      << "<div class=\"charts-row\">\n"
      << "  <div class=\"chart-card\" style=\"margin-bottom:0;\">\n"
      << "    <div class=\"chart-title\">Votos por partido</div>\n"
      << "    <div class=\"legend\" id=\"legBar\"></div>\n"
      << "    <div style=\"position:relative;height:200px;\"><canvas id=\"barChart\"></canvas></div>\n"
      << "  </div>\n"
      << "  <div class=\"chart-card\" style=\"margin-bottom:0;\">\n"
      << "    <div class=\"chart-title\">Distribucion porcentual</div>\n"
      << "    <div class=\"legend\" id=\"legPie\"></div>\n"
      << "    <div style=\"position:relative;height:200px;\"><canvas id=\"pieChart\"></canvas></div>\n"
      << "  </div>\n"
      << "</div>\n"
      << "<div style=\"height:12px;\"></div>\n"

      << "<div class=\"chart-card\">\n"
      << "  <div class=\"chart-title\">Resultados por partido</div>\n"
      << "  <table>\n"
      << "    <thead><tr><th>Color</th><th>Partido</th><th>Votos</th><th>Porcentaje</th><th>Estado</th></tr></thead>\n"
      << "    <tbody id=\"tablaBody\"></tbody>\n"
      << "  </table>\n"
      << "</div>\n"

      << "</div>\n"
      << "<div class=\"footer\">Sistema de Votaciones VOCA 2026 - Reporte generado automaticamente</div>\n"
      << "</div>\n"

      << "<script>\n"
      << "var partidos = " << jsArr.str() << ";\n"
      << "var totalVotos = partidos.reduce(function(s,p){ return s+p.votos; },0);\n"

      << "partidos.forEach(function(p){\n"
      << "  var d1=document.createElement('div'); d1.className='legend-item';\n"
      << "  d1.innerHTML='<div class=\"legend-dot\" style=\"background:'+p.color+'\"></div>'+p.nombre;\n"
      << "  document.getElementById('legBar').appendChild(d1);\n"
      << "  var d2=document.createElement('div'); d2.className='legend-item';\n"
      << "  d2.innerHTML='<div class=\"legend-dot\" style=\"background:'+p.color+'\"></div>'+p.nombre+' '+p.perc+'%';\n"
      << "  document.getElementById('legPie').appendChild(d2);\n"
      << "});\n"

      << "var maxV=Math.max.apply(null,partidos.map(function(p){return p.votos;}));\n"
      << "partidos.forEach(function(p){\n"
      << "  var g=(p.votos===maxV);\n"
      << "  var tr=document.createElement('tr');\n"
      << "  tr.innerHTML='<td><div style=\"width:14px;height:14px;border-radius:50%;background:'+p.color+';margin:auto;\"></div></td>'\n"
      << "    +'<td><strong>'+p.nombre+'</strong></td>'\n"
      << "    +'<td>'+p.votos+'</td>'\n"
      << "    +'<td>'+p.perc+'%</td>'\n"
      << "    +'<td><span class=\"badge\" style=\"background:'+(g?'#2E7D32':'#1565C0')+'\">'+(g?'GANADOR':'Participante')+'</span></td>';\n"
      << "  document.getElementById('tablaBody').appendChild(tr);\n"
      << "});\n"

      << "new Chart(document.getElementById('barChart'),{\n"
      << "  type:'bar',\n"
      << "  data:{ labels:partidos.map(function(p){return p.nombre;}),\n"
      << "    datasets:[{ data:partidos.map(function(p){return p.votos;}),\n"
      << "      backgroundColor:partidos.map(function(p){return p.color;}), borderRadius:4 }]},\n"
      << "  options:{ responsive:true, maintainAspectRatio:false, plugins:{legend:{display:false}},\n"
      << "    scales:{ x:{grid:{display:false}}, y:{beginAtZero:true, ticks:{precision:0}} } }\n"
      << "});\n"

      << "new Chart(document.getElementById('pieChart'),{\n"
      << "  type:'pie',\n"
      << "  data:{ labels:partidos.map(function(p){return p.nombre;}),\n"
      << "    datasets:[{ data:partidos.map(function(p){return p.votos;}),\n"
      << "      backgroundColor:partidos.map(function(p){return p.color;}),\n"
      << "      borderWidth:2, borderColor:'#ffffff' }]},\n"
      << "  options:{ responsive:true, maintainAspectRatio:false, plugins:{ legend:{display:false},\n"
      << "    tooltip:{callbacks:{label:function(ctx){\n"
      << "      return ctx.label+': '+ctx.parsed+' votos ('+(totalVotos>0?(ctx.parsed/totalVotos*100).toFixed(1):0)+'%)';\n"
      << "    }}}}}\n"
      << "});\n"
      << "</script>\n"
      << "</body>\n"
      << "</html>\n";

    std::ofstream file(outPath);
    if (!file.is_open()) return false;
    file << h.str();
    file.close();

#ifdef _WIN32
    std::string cmd = "start \"\" \"" + outPath + "\"";
#else
    std::string cmd = "xdg-open \"" + outPath + "\" &";
#endif
    std::system(cmd.c_str());

    return true;
}
