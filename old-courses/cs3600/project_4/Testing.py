from NeuralNetUtil import buildExamplesFromCarData, buildExamplesFromPenData
from NeuralNet import buildNeuralNet
from math import pow, sqrt


def average(argList):
    return sum(argList) / float(len(argList))


def stDeviation(argList):
    mean = average(argList)
    diffSq = [pow((val - mean), 2) for val in argList]
    return sqrt(sum(diffSq) / len(argList))


penData = buildExamplesFromPenData()


def testPenData(hiddenLayers=[24]):
    return buildNeuralNet(penData, maxItr=200, hiddenLayerList=hiddenLayers)


carData = buildExamplesFromCarData()


def testCarData(hiddenLayers=[16]):
    return buildNeuralNet(carData, maxItr=200, hiddenLayerList=hiddenLayers)


def q5():
    accuracy_list = []
    for _ in range(5):
        nnet, accuracy = testCarData()
        accuracy_list.append(accuracy)
    print(
        f"Car Accuracy: {accuracy_list}.\n"
        + f"Car Max Accuracy: {max(accuracy_list)}.\n"
        + f"Car Avg Accuracy: {average(accuracy_list)}\n"
        + f"Car Accuracy StdDev: {stDeviation(accuracy_list)}."
    )

    accuracy_list = []
    for _ in range(5):
        nnet, accuracy = testPenData()
        accuracy_list.append(accuracy)
    print(
        f"Pen Accuracy: {accuracy_list}.\n"
        + f"Pen Max Accuracy: {max(accuracy_list)}.\n"
        + f"Pen Avg Accuracy: {average(accuracy_list)}\n"
        + f"Pen Accuracy StdDev: {stDeviation(accuracy_list)}."
    )


def q6():
    pentest = []
    for n in range(0, 41, 5):
        accuracy_list = []
        for _ in range(5):
            nnet, accuracy = testPenData([n])
            accuracy_list.append(accuracy)
        print(
            f"Hidden Layers: {n}.\n"
            + f"Pen Accuracy: {accuracy_list}.\n"
            + f"Pen Max Accuracy: {max(accuracy_list)}.\n"
            + f"Pen Avg Accuracy: {average(accuracy_list)}\n"
            + f"Pen Accuracy StdDev: f{stDeviation(accuracy_list)}."
        )
        pentest.append(accuracy_list)
    print(pentest)


q5()
print("\n" * 6)
q6()

