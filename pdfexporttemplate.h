#ifndef PDFEXPORTTEMPLATE_H
#define PDFEXPORTTEMPLATE_H

#pragma once

#include <QString>
#include <QColor>

/**
 * @brief Classe représentant un template d'export PDF
 */
class PdfExportTemplate
{
public:
    PdfExportTemplate(const QString &name = "Default");
    
    QString name() const { return m_name; }
    QString css() const { return m_css; }
    QString headerHtml() const { return m_headerHtml; }
    QString footerHtml() const { return m_footerHtml; }
    
    void setName(const QString &name) { m_name = name; }
    void setCss(const QString &css) { m_css = css; }
    void setHeaderHtml(const QString &html) { m_headerHtml = html; }
    void setFooterHtml(const QString &html) { m_footerHtml = html; }
    
    // Templates prédéfinis
    static PdfExportTemplate defaultTemplate();
    static PdfExportTemplate minimalTemplate();
    static PdfExportTemplate detailedTemplate();
    static PdfExportTemplate colorfulTemplate();
    
    static QList<PdfExportTemplate> availableTemplates();
    
private:
    QString m_name;
    QString m_css;
    QString m_headerHtml;
    QString m_footerHtml;
};

#endif // PDFEXPORTTEMPLATE_H
