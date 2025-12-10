#include "pdfexporttemplate.h"

PdfExportTemplate::PdfExportTemplate(const QString &name)
    : m_name(name)
{
}

PdfExportTemplate PdfExportTemplate::defaultTemplate()
{
    PdfExportTemplate t("Par défaut");
    
    t.setCss(R"(
body { 
    font-family: 'Segoe UI', Arial, sans-serif; 
    margin: 20px;
    color: #2c3e50;
}
h1 { 
    color: #34495e;
    border-bottom: 3px solid #3498db;
    padding-bottom: 10px;
}
.header-info {
    background: #ecf0f1;
    padding: 10px;
    border-radius: 5px;
    margin-bottom: 20px;
}
.task { 
    margin-left: 20px; 
    margin-bottom: 15px;
    padding: 10px;
    border-left: 3px solid #bdc3c7;
}
.task-title { 
    font-weight: bold;
    font-size: 1.1em;
    margin-bottom: 5px;
}
.task-meta { 
    color: #7f8c8d;
    font-size: 0.9em;
    margin: 5px 0;
}
.task-desc {
    margin-top: 5px;
    padding: 5px;
    background: #f9f9f9;
    border-radius: 3px;
}
.tags {
    display: inline-block;
    margin: 2px;
    padding: 2px 8px;
    background: #3498db;
    color: white;
    border-radius: 3px;
    font-size: 0.85em;
}
.priority-very-low { border-left-color: #95a5a6; }
.priority-low { border-left-color: #3498db; }
.priority-medium { border-left-color: #f39c12; }
.priority-high { border-left-color: #e67e22; }
.priority-critical { border-left-color: #e74c3c; }
.status-completed { opacity: 0.6; }
.status-completed .task-title { text-decoration: line-through; }
    )");
    
    t.setHeaderHtml("<h1>{{TITLE}}</h1><div class='header-info'>{{DATE}}</div>");
    t.setFooterHtml("");
    
    return t;
}

PdfExportTemplate PdfExportTemplate::minimalTemplate()
{
    PdfExportTemplate t("Minimal");
    
    t.setCss(R"(
body { 
    font-family: monospace;
    margin: 15px;
    font-size: 10pt;
}
h1 { 
    font-size: 14pt;
    margin-bottom: 10px;
}
.task { 
    margin-left: 15px;
    margin-bottom: 5px;
}
.task-title { font-weight: bold; }
.task-meta { font-size: 9pt; color: #666; }
.status-completed .task-title { text-decoration: line-through; }
    )");
    
    t.setHeaderHtml("<h1>{{TITLE}}</h1><p>{{DATE}}</p>");
    t.setFooterHtml("");
    
    return t;
}

PdfExportTemplate PdfExportTemplate::detailedTemplate()
{
    PdfExportTemplate t("Détaillé");
    
    t.setCss(R"(
body { 
    font-family: Georgia, serif;
    margin: 30px;
    line-height: 1.6;
}
h1 { 
    color: #1a1a1a;
    text-align: center;
    font-size: 24pt;
    margin-bottom: 30px;
}
.header-info {
    text-align: center;
    color: #666;
    margin-bottom: 30px;
    padding-bottom: 20px;
    border-bottom: 2px solid #ddd;
}
.task { 
    margin-left: 25px;
    margin-bottom: 20px;
    padding: 15px;
    background: #fafafa;
    border: 1px solid #e0e0e0;
    border-radius: 5px;
    page-break-inside: avoid;
}
.task-title { 
    font-weight: bold;
    font-size: 14pt;
    margin-bottom: 10px;
    color: #2c3e50;
}
.task-meta {
    margin: 8px 0;
    font-size: 10pt;
}
.meta-label {
    font-weight: bold;
    color: #555;
}
.task-desc {
    margin-top: 10px;
    padding: 10px;
    background: white;
    border-left: 4px solid #3498db;
}
.tags {
    display: inline-block;
    margin: 2px;
    padding: 3px 10px;
    background: #2ecc71;
    color: white;
    border-radius: 4px;
    font-size: 9pt;
}
.priority-critical .task-title { color: #e74c3c; }
.priority-high .task-title { color: #e67e22; }
.status-completed { opacity: 0.5; }
    )");
    
    t.setHeaderHtml(R"(
<h1>📋 {{TITLE}}</h1>
<div class='header-info'>
    <p>Généré le {{DATE}}</p>
    <p>Rapport détaillé des tâches</p>
</div>
    )");
    
    t.setFooterHtml("<div style='text-align: center; margin-top: 30px; color: #999; font-size: 9pt;'>Page générée par ToDoApp</div>");
    
    return t;
}

PdfExportTemplate PdfExportTemplate::colorfulTemplate()
{
    PdfExportTemplate t("Coloré");
    
    t.setCss(R"(
body { 
    font-family: 'Comic Sans MS', cursive, sans-serif;
    margin: 20px;
    background: linear-gradient(to bottom, #fff, #f0f0f0);
}
h1 { 
    color: #fff;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    padding: 20px;
    border-radius: 10px;
    text-align: center;
    text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
}
.header-info {
    background: #fff3cd;
    padding: 15px;
    border-radius: 10px;
    margin: 20px 0;
    border: 2px dashed #ffc107;
}
.task { 
    margin: 15px 0 15px 20px;
    padding: 15px;
    border-radius: 10px;
    box-shadow: 0 2px 5px rgba(0,0,0,0.1);
    background: white;
}
.task-title { 
    font-weight: bold;
    font-size: 1.2em;
    margin-bottom: 8px;
}
.task-meta {
    margin: 5px 0;
    font-size: 0.9em;
}
.tags {
    display: inline-block;
    margin: 2px;
    padding: 4px 12px;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
    border-radius: 20px;
    font-size: 0.85em;
}
.priority-very-low { background: linear-gradient(135deg, #a8edea 0%, #fed6e3 100%); }
.priority-low { background: linear-gradient(135deg, #84fab0 0%, #8fd3f4 100%); }
.priority-medium { background: linear-gradient(135deg, #ffecd2 0%, #fcb69f 100%); }
.priority-high { background: linear-gradient(135deg, #ff9a9e 0%, #fecfef 100%); }
.priority-critical { background: linear-gradient(135deg, #ff0844 0%, #ffb199 100%); }
.status-completed { 
    opacity: 0.6;
    transform: scale(0.98);
}
    )");
    
    t.setHeaderHtml("<h1>🎨 {{TITLE}} 🎨</h1><div class='header-info'>📅 {{DATE}}</div>");
    t.setFooterHtml("");
    
    return t;
}

QList<PdfExportTemplate> PdfExportTemplate::availableTemplates()
{
    return {
        defaultTemplate(),
        minimalTemplate(),
        detailedTemplate(),
        colorfulTemplate()
    };
}
