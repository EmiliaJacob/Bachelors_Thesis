const publishOptions = {
    qos: 1,
    retain: true
}

//handler for user action
const userhandler = {
    set: function(target, prop, value){
        client.publish(prop, value.toString(), publishOptions, function(err){
            if(err){
                console.log('An error ocurred while publishing, object has not been updated', err);
                return false;
            }
            console.log(target);
            target[prop] = value;
            return true;
        })
    },
    //for deletion of data-object
    deleteProperty: function(target, prop){
        client.unsubscribe(prop, function(err){
            if(err){  //if unsubscribe fails
                console.log('unsubscribe failed', err);
                return false
            }
            delColl = [];
            topicParts = prop.split('/');
            for(property in target){
                propertyParts = property.split('/');
                for (var i = 0; i < topicParts.length; i++){
                    if(topicParts[i] == propertyParts[i]){
                        if(topicParts.length-1 == i){
                            delColl.push(property);
                            break;
                        }
                        continue;
                    }
                    if(topicParts[i] == '#'){
                        delColl.push(property);
                        break;
                    }
                    if(topicParts[i] == '+' && (topicParts.length-1 == i)){
                        delColl.push(property);
                        break;
                    }
                    if(topicParts[i] == '+'){continue;}
                    break;
                }
            }
            console.log('removing topics:',delColl);
            delColl.forEach(element => {
                delete target[element];
                deleteTopicRow(element);
            });
            console.log(target);
            return true;
        })
    }
}

//handler for recieved messages
const msghandler = {
    set: function(target, prop, value){
        value = value.toString();
        target[prop] = value;
        console.log('recieved pub message:', prop, ':', value);
        var loc = checkRowDatasetTableExists(prop);
        if(loc != -1){
            replaceRowDatasetTable(value, loc);
            console.log(target);
            return;
        }
        addRowDatasetTable(prop, value);
        console.log(target);
    }
}

function deleteTopicRow(topic){
    var row = checkRowDatasetTableExists(topic);
    if(row != -1){
        deleteDatasetTable(row);   
    }
} 
