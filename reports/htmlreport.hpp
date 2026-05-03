#pragma once
#include <vector>

// Genera el archivo HTML del informe de resultados y lo abre en el navegador.
// percVec  : porcentajes de cada partido (el ultimo valor es abstenciones)
// quanVec  : cantidades absolutas de cada partido (el ultimo valor es abstenciones)
// Devuelve true si el HTML se creo y abrio con exito, false si hubo error.
bool htmlreport(std::vector<double> percVec, std::vector<int> quanVec);
