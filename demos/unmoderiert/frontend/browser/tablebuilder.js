const datasetDiv = document.querySelector("div.dataset")
let datasetHead = ["Topic", "Message"]

const sanitize = (string) => {
    const map = {
        '&': '&amp;',
        '<': '&lt;',
        '>': '&gt;',
        '"': '&quot;',
        "'": '&#x27;',
    };
    const reg = /[&<>"']/ig;
    return string.replace(reg, (match)=>(map[match]));
}

const createDatasetTable = () => {
    while (datasetDiv.firstChild) datasetDiv.removeChild(datasetDiv.firstChild)

    let datasetTable = document.createElement('table')
    datasetTable.className = 'datasetTable'

    let datasetTableHead = document.createElement('thead')
    datasetTableHead.className = 'datasetTableHead'

    let datasetTableBody = document.createElement('tbody')

    let datasetTableHeadRow = document.createElement('tr')
    datasetTableHeadRow.className = 'datasetTableHeadRow'

    datasetHead.forEach(header => {
        let datasetHeader = document.createElement('th')
        datasetHeader.innerText = header
        datasetTableHeadRow.append(datasetHeader)
    })

    datasetTableHead.append(datasetTableHeadRow)
    datasetTable.append(datasetTableHead, datasetTableBody)

    datasetDiv.append(datasetTable)
}

const addRowDatasetTable = (topic, message) => {
    const datasetTable = document.querySelector('tbody');

    let datasetTableRow = document.createElement('tr');
    
    let topicCell = document.createElement('td');
    topicCell.innerText = sanitize(topic);

    let messageCell = document.createElement('td');
    messageCell.innerText = sanitize(message);

    datasetTableRow.append(topicCell, messageCell);
    datasetTable.append(datasetTableRow);
}

const checkRowDatasetTableExists = (topic) => {
    var datasetTable = document.querySelector('tbody')
    var datasetRows = datasetTable.getElementsByTagName('tr')

    for(i=0; i<datasetRows.length; i++){
        var datasetTopicCell = datasetRows[i].getElementsByTagName('td')[0]
        if(datasetTopicCell){
            if(datasetTopicCell.textContent == sanitize(topic)){
                return i
            }
        }
    }
    return -1
}

const replaceRowDatasetTable = (message, pos) => {
    var datasetTable = document.querySelector('tbody')
    var datasetRows = datasetTable.getElementsByTagName('tr')
    var datasetMessageCell = datasetRows[pos].getElementsByTagName('td')[1]
    datasetMessageCell.textContent = sanitize(message)
}

const deleteDatasetTable = (row) => {
    var datasetTable = document.querySelector('tbody')
    var datasetRows = datasetTable.getElementsByTagName('tr')
    datasetRows[row].remove()
}